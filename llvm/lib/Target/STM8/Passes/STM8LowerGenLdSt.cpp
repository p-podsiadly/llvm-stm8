#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8InstrInfo.h"
#include "STM8Passes.h"
#include "STM8RegisterInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#define DEBUG_TYPE "stm8-lower-gen-ld-st"
#define STM8_PASS_NAME "STM8 Lower Generic Loads/Stores"

using namespace llvm;

namespace {

class STM8LowerGenLdSt : public MachineFunctionPass {
public:
  static char ID;

  STM8LowerGenLdSt() : MachineFunctionPass(ID) {
    initializeSTM8LowerGenLdStPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return STM8_PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const TargetInstrInfo *CurTII = nullptr;
  MachineRegisterInfo *CurMRI = nullptr;

  bool lowerInstr(MachineInstr &MI) const;

  Register insertCopyToRC(MachineInstr &BeforeMI, Register Src,
                          const TargetRegisterClass *NewRC) const;

  static const TargetRegisterClass *
  getComplementalRegClass(const TargetRegisterClass *RC);

  static bool isConcreteRegClass(const TargetRegisterClass *RC) {
    return (RC == &STM8::XREGRegClass) || (RC == &STM8::YREGRegClass);
  }
};

} // namespace

char STM8LowerGenLdSt::ID = {};

bool STM8LowerGenLdSt::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "***** " << STM8_PASS_NAME << " *****\n");

  CurTII = MF.getSubtarget().getInstrInfo();
  CurMRI = &MF.getRegInfo();

  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      unsigned Opcode = MI.getOpcode();
      switch (Opcode) {
      case STM8::GenLDWirr:
      case STM8::GenLDWird8r:
        Changed |= lowerInstr(MI);
        break;
      default:
        break;
      }
    }
  }

  CurTII = nullptr;
  CurMRI = nullptr;

  if (Changed) {
    LLVM_DEBUG(dbgs() << "  *** Function " << MF.getName()
                      << " after lowering ***\n\n");
    LLVM_DEBUG(MF.print(dbgs()));
  }

  return Changed;
}

bool STM8LowerGenLdSt::lowerInstr(MachineInstr &MI) const {
  unsigned Opcode = MI.getOpcode();

  std::array<unsigned, 2> RegOpIndices = {};
  std::array<unsigned, 2> NewOpcodes = {};
  switch (Opcode) {
  case STM8::GenLDWirr:
    RegOpIndices = {0, 1};
    NewOpcodes = {STM8::LDWixy, STM8::LDWiyx};
    break;
  case STM8::GenLDWird8r:
    RegOpIndices = {0, 2};
    NewOpcodes = {STM8::LDWixd8y, STM8::LDWiyd8x};
    break;
  default:
    llvm_unreachable("Unexected opcode in STM8 Gen Load/Store lowering!");
  }

  Register Reg0 = MI.getOperand(RegOpIndices[0]).getReg();
  Register Reg1 = MI.getOperand(RegOpIndices[1]).getReg();

  const TargetRegisterClass *RC0 = CurMRI->getRegClass(Reg0);
  const TargetRegisterClass *RC1 = CurMRI->getRegClass(Reg1);

  if (isConcreteRegClass(RC0) && isConcreteRegClass(RC1)) {
    // Case 1: both registers are assigned to either X or Y

    if (RC0 == RC1) {
      RC1 = getComplementalRegClass(RC0);
      Register NewReg1 = insertCopyToRC(MI, Reg1, RC1);
      MI.getOperand(RegOpIndices[1]).setReg(NewReg1);
    } else {
      return false;
    }
  } else if (isConcreteRegClass(RC0) && !isConcreteRegClass(RC1)) {
    // Case 2: only reg #0 is assigned to X or Y

    RC1 = getComplementalRegClass(RC0);
    Register NewReg1 = insertCopyToRC(MI, Reg1, RC1);
    MI.getOperand(RegOpIndices[1]).setReg(NewReg1);
  } else if (!isConcreteRegClass(RC0) && isConcreteRegClass(RC1)) {
    // Case 3: only reg #1 is assigned to X or Y

    RC0 = getComplementalRegClass(RC1);
    Register NewReg0 = insertCopyToRC(MI, Reg0, RC0);
    MI.getOperand(RegOpIndices[0]).setReg(NewReg0);
  } else {
    // Case 4: neither register is assigned to X or Y

    RC0 = &STM8::YREGRegClass;
    RC1 = getComplementalRegClass(RC0);

    Register NewReg0 = insertCopyToRC(MI, Reg0, RC0);
    Register NewReg1 = insertCopyToRC(MI, Reg1, RC1);

    MI.getOperand(RegOpIndices[0]).setReg(NewReg0);
    MI.getOperand(RegOpIndices[1]).setReg(NewReg1);
  }

  unsigned NewOpc =
      STM8::XREGRegClass.hasSubClassEq(RC0) ? NewOpcodes[0] : NewOpcodes[1];
  MI.setDesc(CurTII->get(NewOpc));

  LLVM_DEBUG(dbgs() << " * Lowered " << CurTII->getName(Opcode) << " to ");
  LLVM_DEBUG(MI.print(dbgs()));

  return true;
}

Register
STM8LowerGenLdSt::insertCopyToRC(MachineInstr &BeforeMI, Register Src,
                                 const TargetRegisterClass *NewRC) const {
  Register NewSrc = CurMRI->createVirtualRegister(NewRC);
  BuildMI(*BeforeMI.getParent(), BeforeMI.getIterator(), BeforeMI.getDebugLoc(),
          CurTII->get(STM8::COPY), NewSrc)
      .addReg(Src);
  return NewSrc;
}

const TargetRegisterClass *
STM8LowerGenLdSt::getComplementalRegClass(const TargetRegisterClass *RC) {
  if (RC == &STM8::XREGRegClass) {
    return &STM8::YREGRegClass;
  }

  assert(RC == &STM8::YREGRegClass);
  return &STM8::XREGRegClass;
}

INITIALIZE_PASS_BEGIN(STM8LowerGenLdSt, "stm8-lower-gen-ld-st", STM8_PASS_NAME,
                      false, false)
INITIALIZE_PASS_END(STM8LowerGenLdSt, DEBUG_TYPE, STM8_PASS_NAME, false, false)

MachineFunctionPass *llvm::createSTM8LowerGenLdStPass() {
  return new STM8LowerGenLdSt();
}