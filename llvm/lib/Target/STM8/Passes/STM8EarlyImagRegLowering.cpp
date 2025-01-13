#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8Passes.h"
#include "STM8RegisterInfo.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/InitializePasses.h"
#include <unordered_map>

#define STM8_PASS_NAME "STM8 Early Imaginary Register Lowering"
#define DEBUG_TYPE "stm8-imag-reg-lowering"

using namespace llvm;

namespace {

struct ImagRegClass {
  struct InstrInfo {
    unsigned Opcode = 0;
    unsigned LoweredOpcode = 0;
  };

  unsigned Size = 0;
  const TargetRegisterClass *TargetRC = nullptr;

  unsigned StorePhysRegOpcode = 0;
  unsigned LoadPhysRegOpcode = 0;
  const TargetRegisterClass *PhysRC = nullptr;
  ArrayRef<const InstrInfo> InstrInfos;

  unsigned getLoweredOpcode(unsigned Opcode) const {
    for (const auto &II : InstrInfos) {
      if (II.Opcode == Opcode) {
        return II.LoweredOpcode;
      }
    }

    return 0;
  }
};

class STM8EarlyImagRegLowering : public MachineFunctionPass {
public:
  static char ID;

  STM8EarlyImagRegLowering() : MachineFunctionPass(ID) {
    initializeSTM8EarlyImagRegLoweringPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return STM8_PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  MachineFunction *CurMF = nullptr;
  const TargetInstrInfo *CurTII = nullptr;
  std::unordered_map<unsigned, int> ImagRegToStackSlot;

  void runOnMBB(MachineBasicBlock &MBB);

  void lowerInstr(MachineBasicBlock &MBB, MachineInstr &MI,
                  unsigned ImagRegOpNum, const ImagRegClass *IRClass);

  // Lowers PHI instruction which defines an imaginary register.
  void lowerPhiDef(MachineBasicBlock &MBB, MachineInstr &MI,
                   const ImagRegClass *IRClass);

  // Lowers PHI instruction which defines a physical register and uses an
  // imaginary register.
  void lowerPhiUse(MachineBasicBlock &MBB, MachineInstr &MI,
                   const ImagRegClass *IRClass);

  void insertImagRegCopy(MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MII, const DebugLoc &DL,
                         int DstFI, int SrcFI, const ImagRegClass *IRClass);

  bool isImagReg(const Register &Reg) const;
  const ImagRegClass *getImagRegClass(const Register &Reg) const;

  int getStackSlot(const Register &Reg);
  std::optional<int> tryGetStackSlot(const Register &Reg) const;
  void assignStackSlot(const Register &Reg, int FI);

  std::string getMBBName(const MachineBasicBlock &MBB) const;
};

MachineBasicBlock::iterator findMBBEndInsertPos(MachineBasicBlock &MBB) {
  MachineBasicBlock::iterator InsertPos = MBB.getLastNonDebugInstr();
  while ((InsertPos != MBB.begin()) && InsertPos->isTerminator()) {
    InsertPos = std::prev(InsertPos);
  }

  // We want to insert new instructions immediately *after* InsertPos.
  return std::next(InsertPos);
}

} // namespace

char STM8EarlyImagRegLowering::ID = {};

bool STM8EarlyImagRegLowering::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "***** STM8 Early Imaginary Register Lowering *****\n");
  LLVM_DEBUG(dbgs() << " **** Function: " << MF.getName() << " ****\n");

  CurMF = &MF;
  CurTII = MF.getSubtarget().getInstrInfo();
  ImagRegToStackSlot.clear();

  for (MachineBasicBlock &MBB : MF) {
    runOnMBB(MBB);
  }

  LLVM_DEBUG(dbgs() << " **** Function after lowering: ****\n");
  LLVM_DEBUG(MF.print(dbgs()));

  CurMF = nullptr;
  CurTII = nullptr;
  ImagRegToStackSlot.clear();

  return true;
}

void STM8EarlyImagRegLowering::runOnMBB(MachineBasicBlock &MBB) {

  LLVM_DEBUG(dbgs() << "* Lowering imaginary registers in " << getMBBName(MBB)
                    << "\n");

  for (MachineInstr &MI : make_early_inc_range(MBB.instrs())) {
    // Find the first imaginary register operand in the instruction
    const ImagRegClass *IRClass = nullptr;
    unsigned ImagRegOpNum = 0;
    for (unsigned OpNum = 0; OpNum < MI.getNumExplicitOperands(); ++OpNum) {
      const MachineOperand &Op = MI.getOperand(OpNum);
      if (!Op.isReg() || Op.getReg().isPhysical()) {
        continue;
      }

      IRClass = getImagRegClass(Op.getReg());
      if (IRClass) {
        ImagRegOpNum = OpNum;
        break;
      }
    }

    if (IRClass) {
      lowerInstr(MBB, MI, ImagRegOpNum, IRClass);
    }
  }
}

void STM8EarlyImagRegLowering::lowerInstr(MachineBasicBlock &MBB,
                                          MachineInstr &MI, unsigned OpNum,
                                          const ImagRegClass *IRClass) {
  unsigned Opcode = MI.getOpcode();

  if ((Opcode == STM8::COPY) && (OpNum == 0)) {
    // Case 1: Copy to imaginary register

    const MachineOperand &SrcOp = MI.getOperand(1);
    Register DstReg = MI.getOperand(0).getReg();
    Register SrcReg = SrcOp.getReg();

    const ImagRegClass *RhsIRClass = getImagRegClass(SrcReg);
    if (RhsIRClass) {
      // Copy from one imag reg to another imag reg
      assert((IRClass == RhsIRClass) && "Copies between different imaginary "
                                        "register classes are not allowed");

      auto DstFI = tryGetStackSlot(DstReg);
      auto SrcFI = tryGetStackSlot(SrcReg);

      LLVM_DEBUG(dbgs() << " * Deleting copy between imaginary registers: ");
      LLVM_DEBUG(MI.print(dbgs()));

      if (!SrcFI.has_value()) {
        // SrcReg does not have a stack slot

        if (!DstFI.has_value()) {
          DstFI = getStackSlot(DstReg);
        }

        assignStackSlot(SrcReg, DstFI.value());
      } else if (!DstFI.has_value()) {
        // SrcReg has stack slot, DstReg does not
        assignStackSlot(DstReg, SrcFI.value());
      } else if (SrcFI != DstFI) {
        // Both register already have different stack slots
        insertImagRegCopy(MBB, MI.getIterator(), MI.getDebugLoc(),
                          DstFI.value(), SrcFI.value(), IRClass);
      }

      MI.removeFromParent();
    } else {
      // Copy from physical reg to imag reg
      int FI = getStackSlot(DstReg);

      LLVM_DEBUG(dbgs() << " * Replacing ");
      LLVM_DEBUG(MI.print(dbgs(), true, false, false, false, CurTII));
      LLVM_DEBUG(dbgs() << " with store to stack\n");

      BuildMI(MBB, MI.getIterator(), MI.getDebugLoc(),
              CurTII->get(IRClass->StorePhysRegOpcode))
          .addFrameIndex(FI)
          .addImm(0)
          .addReg(SrcReg, getRegState(SrcOp));

      MI.removeFromParent();
    }
  } else if ((Opcode == STM8::COPY) && (OpNum == 1)) {
    // Case 2: Copy from imaginary register to physical register

    Register DstReg = MI.getOperand(0).getReg();
    Register SrcReg = MI.getOperand(1).getReg();

    int FI = getStackSlot(SrcReg);

    LLVM_DEBUG(dbgs() << " * Replacing ");
    LLVM_DEBUG(MI.print(dbgs(), true, false, false, false, CurTII));
    LLVM_DEBUG(dbgs() << " with load from stack\n");

    BuildMI(MBB, MI.getIterator(), MI.getDebugLoc(),
            CurTII->get(IRClass->LoadPhysRegOpcode), DstReg)
        .addFrameIndex(FI)
        .addImm(0);

    MI.removeFromParent();
  } else if (Opcode == STM8::PHI) {
    // Case 3: PHI is removed. If all incoming registers have the same stack
    // slot then we reuse it. Otherwise, we choose stack slot of the first
    // incoming register and insert imag reg copy instructions as necessary into
    // predecessors.
    if (OpNum == 0) {
      lowerPhiDef(MBB, MI, IRClass);
      MI.removeFromParent();
    } else {
      lowerPhiUse(MBB, MI, IRClass);
    }
  } else if ((Opcode == STM8::KILL) || (Opcode == STM8::IMPLICIT_DEF)) {
    // Case 3: KILL and IMPLICIT_DEF are relevant only to registers.
    MI.removeFromParent();
  } else {
    // Case 4: lower the instruction by changing the opcode and replacing the
    // operand with frame index

    unsigned NewOpcode = IRClass->getLoweredOpcode(Opcode);

    LLVM_DEBUG(dbgs() << " * Lowering ");
    LLVM_DEBUG(MI.print(dbgs(), true, false, false, false, CurTII));
    LLVM_DEBUG(dbgs() << " to " << CurTII->getName(NewOpcode) << "\n");
    assert(NewOpcode && "Unexpected imaginary register instruction!");

    int FI = getStackSlot(MI.getOperand(OpNum).getReg());

    MI.setDesc(CurTII->get(NewOpcode));
    MI.insert(MI.operands_begin() + OpNum, MachineOperand::CreateFI(FI));
    MI.getOperand(OpNum + 1).ChangeToImmediate(0);
  }
}

void STM8EarlyImagRegLowering::lowerPhiDef(MachineBasicBlock &MBB,
                                           MachineInstr &MI,
                                           const ImagRegClass *IRClass) {
  Register DstReg = MI.getOperand(0).getReg();

  unsigned FirstUse = MI.getNumExplicitDefs();
  unsigned NumUses = MI.getNumExplicitOperands() - FirstUse;

  int FI = getStackSlot(DstReg);

  LLVM_DEBUG(dbgs() << " * PHI def of %" << DstReg.virtRegIndex()
                    << ", mapped to stack slot #" << FI << "\n");

  for (unsigned UseIdx = 0; UseIdx < NumUses; UseIdx += 2) {
    Register InReg = MI.getOperand(FirstUse + UseIdx).getReg();
    MachineBasicBlock *InMBB = MI.getOperand(FirstUse + UseIdx + 1).getMBB();

    if (isImagReg(InReg)) {
      // Incoming register is also an imaginary register.

      int SrcFI = getStackSlot(InReg);
      LLVM_DEBUG(dbgs() << "   * Imaginary source register %"
                        << InReg.virtRegIndex() << ", stack slot #" << SrcFI
                        << "\n");

      if (FI != SrcFI) {
        // Need to insert a copy to imag register in InMBB.
        MachineBasicBlock::iterator InsertPos = findMBBEndInsertPos(*InMBB);
        insertImagRegCopy(*InMBB, InsertPos, MI.getDebugLoc(), FI, SrcFI,
                          IRClass);
      }
    } else {
      // Incoming register is a physical register.

      LLVM_DEBUG(dbgs() << "   * Virtual source register %"
                        << InReg.virtRegIndex()
                        << ", inserting store to stack in "
                        << getMBBName(*InMBB) << "\n");

      MachineBasicBlock::iterator InsertPos = findMBBEndInsertPos(*InMBB);
      BuildMI(*InMBB, InsertPos, MI.getDebugLoc(),
              CurTII->get(IRClass->StorePhysRegOpcode))
          .addFrameIndex(FI)
          .addImm(0)
          .addReg(InReg);
    }
  }
}

void STM8EarlyImagRegLowering::lowerPhiUse(MachineBasicBlock &MBB,
                                           MachineInstr &MI,
                                           const ImagRegClass *IRClass) {
  Register DstReg = MI.getOperand(0).getReg();
  assert(!isImagReg(DstReg) && "Unexpected PHI with imaginary reg def");

  unsigned FirstUse = MI.getNumExplicitDefs();
  unsigned NumUses = MI.getNumExplicitOperands() - FirstUse;
  for (unsigned UseIdx = 0; UseIdx < NumUses; UseIdx += 2) {
    MachineOperand &InRegOp = MI.getOperand(UseIdx);
    if (!isImagReg(InRegOp.getReg())) {
      continue;
    }

    MachineBasicBlock *InMBB = MI.getOperand(UseIdx + 1).getMBB();

    int FI = getStackSlot(InRegOp.getReg());
    Register InReg = CurMF->getRegInfo().createVirtualRegister(IRClass->PhysRC);

    auto InsertPos = findMBBEndInsertPos(*InMBB);
    BuildMI(*InMBB, InsertPos, MI.getDebugLoc(),
            CurTII->get(IRClass->LoadPhysRegOpcode), InReg)
        .addFrameIndex(FI)
        .addImm(0);

    InRegOp.setReg(InReg);
  }
}

void STM8EarlyImagRegLowering::insertImagRegCopy(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MII, const DebugLoc &DL,
    int DstFI, int SrcFI, const ImagRegClass *IRClass) {

  LLVM_DEBUG(dbgs() << "   * Inserting copy from stack slot #" << SrcFI
                    << " to #" << DstFI << " in " << getMBBName(MBB) << "\n");

  Register TempReg = CurMF->getRegInfo().createVirtualRegister(IRClass->PhysRC);

  BuildMI(MBB, MII, DL, CurTII->get(IRClass->LoadPhysRegOpcode), TempReg)
      .addFrameIndex(SrcFI)
      .addImm(0);

  BuildMI(MBB, MII, DL, CurTII->get(IRClass->StorePhysRegOpcode))
      .addFrameIndex(DstFI)
      .addImm(0)
      .addReg(TempReg);
}

bool STM8EarlyImagRegLowering::isImagReg(const Register &Reg) const {
  return getImagRegClass(Reg) != nullptr;
}

const ImagRegClass *
STM8EarlyImagRegLowering::getImagRegClass(const Register &Reg) const {
  static const ImagRegClass::InstrInfo BRegInstrInfos[] = {
      {STM8::PseudoADCab, STM8::ADCaispd8},
      {STM8::PseudoSBCab, STM8::SBCaispd8},
      {STM8::PseudoADDab, STM8::ADDaispd8},
      {STM8::PseudoSUBab, STM8::SUBaispd8},
      {STM8::PseudoANDab, STM8::ANDaispd8},
      {STM8::PseudoORab, STM8::ORaispd8},
      {STM8::PseudoXORab, STM8::XORaispd8},
      {STM8::PseudoCPab, STM8::CPaispd8},
      {STM8::PseudoTNZb, STM8::TNZispd8},
      {STM8::GenTNZr, STM8::TNZispd8},
      {STM8::PseudoLDab, STM8::LDaispd8},
      {STM8::PseudoLDba, STM8::LDispd8a},
      {STM8::TNZr_JR, STM8::TNZispd8_JR},
      {STM8::CPab_JR, STM8::CPaispd8_JR}};

  static const ImagRegClass BRegClass = {/*Size=*/1,
                                         /*TargetRC=*/&STM8::BREGRegClass,
                                         /*StorePhysRegOpcode*/ STM8::LDispd8a,
                                         /*LoadPhysRegOpcode=*/STM8::LDaispd8,
                                         /*PhysRC=*/&STM8::AREGRegClass,
                                         /*InstrInfos=*/BRegInstrInfos};

  static const ImagRegClass::InstrInfo ZRegInstrInfos[] = {
      {STM8::GenADDWrz, STM8::GenADDWrispd8},
      {STM8::GenSUBWrz, STM8::GenSUBWrispd8},
      {STM8::PseudoADCWrz, STM8::PseudoADCWrispd8},
      {STM8::PseudoCPWxz, STM8::CPWxispd8},
      {STM8::GenLDWrz, STM8::GenLDWrispd8},
      {STM8::GenLDWzr, STM8::GenLDWispd8r},
      {STM8::CPWxz_JR, STM8::CPWxispd8_JR},
  };

  static const ImagRegClass ZRegClass = {
      /*Size=*/2,
      /*TargetRC=*/&STM8::ZREGRegClass,
      /*StorePhysRegOpcode*/ STM8::GenLDWispd8r,
      /*LoadPhysRegOpcode=*/STM8::GenLDWrispd8,
      /*PhysRC=*/&STM8::XYRegClass,
      /*InstrInfos=*/ZRegInstrInfos};

  if (!Reg.isVirtual()) {
    return nullptr;
  }

  const TargetRegisterClass *TRC = CurMF->getRegInfo().getRegClass(Reg);
  if (TRC == &STM8::BREGRegClass) {
    return &BRegClass;
  } else if (TRC == &STM8::ZREGRegClass) {
    return &ZRegClass;
  }

  return nullptr;
}

int STM8EarlyImagRegLowering::getStackSlot(const Register &Reg) {
  auto OptFI = tryGetStackSlot(Reg);
  if (OptFI.has_value()) {
    return OptFI.value();
  }

  const auto *IRClass = getImagRegClass(Reg);
  assert(IRClass);

  MachineFrameInfo &FrameInfo = CurMF->getFrameInfo();
  int FI = FrameInfo.CreateSpillStackObject(IRClass->Size, Align(1));
  assignStackSlot(Reg, FI);
  return FI;
}

std::optional<int>
STM8EarlyImagRegLowering::tryGetStackSlot(const Register &Reg) const {
  auto Found = ImagRegToStackSlot.find(Reg.id());
  if (Found != ImagRegToStackSlot.end()) {
    return Found->second;
  }

  return std::nullopt;
}

void STM8EarlyImagRegLowering::assignStackSlot(const Register &Reg, int FI) {
  assert(0 == ImagRegToStackSlot.count(Reg.id()));

  ImagRegToStackSlot.insert(std::make_pair(Reg.id(), FI));
}

std::string
STM8EarlyImagRegLowering::getMBBName(const MachineBasicBlock &MBB) const {
  std::string BBNumStr = "bb." + utostr(MBB.getNumber());

  StringRef Name = MBB.getName();
  if (!Name.empty()) {
    return BBNumStr + "." + Name.str();
  }

  return BBNumStr;
}

INITIALIZE_PASS_BEGIN(STM8EarlyImagRegLowering, "stm8-imag-reg-lowering",
                      STM8_PASS_NAME, false, false)
INITIALIZE_PASS_END(STM8EarlyImagRegLowering, DEBUG_TYPE, STM8_PASS_NAME, false,
                    false)

MachineFunctionPass *llvm::createSTM8EarlyImagRegLoweringPass() {
  return new STM8EarlyImagRegLowering();
}
