#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8Passes.h"
#include "llvm/CodeGen/MachineFunctionPass.h"

#define DEBUG_TYPE "stm8-late-intr-elim"
#define STM8_PASS_NAME "STM8 Late Instruction Elimination"

using namespace llvm;

namespace {

enum CCBit {
  CC_V = 0x1,
  CC_N = 0x2,
  CC_Z = 0x4,
  CC_C = 0x8,
};

/// Returns bits affected by instruction MI.
///
/// SrcReg is set to point to a register based on which CC bits are set.
/// SrcReg.isValid() = false means that CC is not set based on a register.
unsigned getAffectedCCBits(const MachineInstr &MI, Register &SrcReg) {
  unsigned Bits = 0;
  SrcReg = {};

  for (const MachineOperand &MO : MI.implicit_operands()) {
    if (!MO.isDef() || !MO.isReg() || !MO.getReg().isPhysical()) {
      // We're interested only in phys reg defs
      continue;
    }

    MCRegister DefReg = MO.getReg().asMCReg();
    if (DefReg == STM8::CCv) {
      Bits |= CC_V;
    } else if (DefReg == STM8::CCn) {
      Bits |= CC_N;
    } else if (DefReg == STM8::CCz) {
      Bits |= CC_Z;
    } else if (DefReg == STM8::CCc) {
      Bits |= CC_C;
    }
  }

  if (!Bits || (MI.getNumOperands() == 0)) {
    return Bits;
  }

  // On STM8, instructions which define the first operand will also set CC based
  // on it.
  const MachineOperand &Op0 = MI.getOperand(0);
  if (Op0.isDef() && Op0.isReg()) {
    SrcReg = Op0.getReg();
    return Bits;
  }

  unsigned Opcode = MI.getOpcode();
  switch (Opcode) {
  case STM8::GenTNZr:
  case STM8::TNZa:
  case STM8::GenTNZWr:
    SrcReg = Op0.getReg();
    break;
  case STM8::LDm8a:
  case STM8::LDm16a:
  case STM8::LDixa:
  case STM8::LDiya:
  case STM8::LDim8a:
  case STM8::LDim16a:
  case STM8::GenLDWirr:
  case STM8::LDWixy:
  case STM8::LDWiyx:
  case STM8::GenLDWm16r:
  case STM8::LDWm16x:
  case STM8::LDWm16y:
    SrcReg = MI.getOperand(1).getReg();
    break;
  case STM8::LDixd8a:
  case STM8::LDixd16a:
  case STM8::LDispd8a:
  case STM8::GenLDWird8r:
  case STM8::LDWixd8y:
  case STM8::LDWixd16y:
  case STM8::LDWiyd8x:
  case STM8::LDWiyd16x:
  case STM8::GenLDWispd8r:
  case STM8::LDWispd8x:
  case STM8::LDWispd8y:
    SrcReg = MI.getOperand(2).getReg();
    break;
  }

  return Bits;
}

class STM8LateInstrElimination : public MachineFunctionPass {
public:
  static char ID;

  STM8LateInstrElimination() : MachineFunctionPass(ID) {
    initializeSTM8LateInstrEliminationPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return STM8_PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  MachineFunction *CurMF = nullptr;

  bool tryEliminateCompare(MachineInstr &MI, MachineInstr &PrevMI) const;
};

} // namespace

char STM8LateInstrElimination::ID = {};

bool STM8LateInstrElimination::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "***** " << STM8_PASS_NAME << " *****\n");

  CurMF = &MF;

  bool Modified = false;
  for (MachineBasicBlock &MBB : MF) {
    MachineInstr *PrevMI = nullptr;
    for (MachineInstr &MI : make_early_inc_range(MBB)) {
      if (!PrevMI) {
        PrevMI = &MI;
        continue;
      }

      unsigned Opcode = MI.getOpcode();
      bool RemoveMI = false;
      switch (Opcode) {
      case STM8::CPai8:
      case STM8::GenTNZr:
      case STM8::TNZa:
      case STM8::GenCPWri16:
      case STM8::GenTNZWr:
        RemoveMI = tryEliminateCompare(MI, *PrevMI);
        break;
      default:
        break;
      }

      if (RemoveMI) {
        LLVM_DEBUG(dbgs() << " * Removing ");
        LLVM_DEBUG(MI.print(dbgs()));

        MI.removeFromBundle();
        Modified = true;
      } else {
        PrevMI = &MI;
      }
    }
  }

  CurMF = nullptr;

  return Modified;
}

bool STM8LateInstrElimination::tryEliminateCompare(MachineInstr &MI,
                                                   MachineInstr &PrevMI) const {
  Register PrevSrcReg;
  unsigned PrevCCBits = getAffectedCCBits(PrevMI, PrevSrcReg);
  if (!PrevCCBits || !PrevSrcReg.isValid()) {
    return false;
  }

  Register SrcReg;
  unsigned CCBits = getAffectedCCBits(MI, SrcReg);
  if (!CCBits || !SrcReg.isValid()) {
    return false;
  }

  return (PrevSrcReg == SrcReg) && ((PrevCCBits & CCBits) == CCBits);
}

INITIALIZE_PASS_BEGIN(STM8LateInstrElimination, "stm8-late-instr-elim",
                      STM8_PASS_NAME, false, false)
INITIALIZE_PASS_END(STM8LateInstrElimination, DEBUG_TYPE, STM8_PASS_NAME, false,
                    false)

MachineFunctionPass *llvm::createSTM8LateInstrEliminationPass() {
  return new STM8LateInstrElimination();
}
