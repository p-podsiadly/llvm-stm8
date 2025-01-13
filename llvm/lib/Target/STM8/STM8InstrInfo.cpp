#include "STM8InstrInfo.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8ISelLowering.h"
#include "STM8RegisterInfo.h"
#include <llvm/CodeGen/ISDOpcodes.h>
#include <llvm/CodeGen/MachineFrameInfo.h>

#define GET_INSTRINFO_CTOR_DTOR
#include "STM8GenInstrInfo.inc"

#define DEBUG_TYPE "stm8-instr-info"

namespace llvm {

STM8InstrInfo::STM8InstrInfo()
    : STM8GenInstrInfo(STM8::ADJCALLSTACKDOWN, STM8::ADJCALLSTACKUP, 0,
                       STM8::RET) {}

void STM8InstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool isKill, int FrameIndex, const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI, Register VReg) const {
  MachineFunction *MF = MBB.getParent();
  assert(MF);

  MachineFrameInfo &MFI = MF->getFrameInfo();

  MachineMemOperand *MO = MF->getMachineMemOperand(
      MachinePointerInfo::getFixedStack(*MF, FrameIndex),
      MachineMemOperand::MOStore, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlign(FrameIndex));

  unsigned OpCode = 0;
  if (STM8::AREGRegClass.hasSubClassEq(RC)) {
    OpCode = STM8::LDispd8a;
  } else if (STM8::BREGRegClass.hasSubClassEq(RC)) {
    OpCode = STM8::PseudoLDispd8b;
  } else if (STM8::XYRegClass.hasSubClassEq(RC)) {
    OpCode = STM8::GenLDWispd8r;
  } else if (STM8::ZREGRegClass.hasSubClassEq(RC)) {
    OpCode = STM8::PseudoLDWispd8z;
  } else {
    llvm_unreachable("Unexpected source register");
  }

  BuildMI(MBB, MI, DebugLoc(), get(OpCode))
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addReg(SrcReg)
      .addMemOperand(MO);
}

void STM8InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI,
                                         Register VReg) const {
  MachineFunction &MF = *MBB.getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOLoad, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlign(FrameIndex));

  unsigned OpCode = 0;
  if (STM8::AREGRegClass.hasSubClassEq(RC)) {
    OpCode = STM8::LDaispd8;
  } else if (STM8::BREGRegClass.hasSubClassEq(RC)) {
    OpCode = STM8::PseudoLDbispd8;
  } else if (STM8::XYRegClass.hasSubClassEq(RC)) {
    OpCode = STM8::GenLDWrispd8;
  } else if (STM8::ZREGRegClass.hasSubClassEq(RC)) {
    OpCode = STM8::PseudoLDWzispd8;
  } else {
    llvm_unreachable("Unexpected destination register");
  }

  BuildMI(MBB, MI, DebugLoc(), get(OpCode), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(MO);
}

unsigned STM8InstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                            int &FrameIndex) const {
  unsigned Opcode = MI.getOpcode();

  auto setFIAndReturnDstReg = [&](const MachineOperand &FIOp) -> unsigned {
    const MachineOperand &RegOp = MI.getOperand(0);
    if (RegOp.isReg() && FIOp.isFI()) {
      FrameIndex = FIOp.getIndex();
      return RegOp.getReg().id();
    }

    return 0;
  };

  switch (Opcode) {
  case STM8::LDaispd8:
  case STM8::PseudoLDbispd8:
  case STM8::GenLDWrispd8:
  case STM8::PseudoLDWzispd8:
    return setFIAndReturnDstReg(MI.getOperand(2));
  default:
    break;
  }

  return 0;
}

unsigned STM8InstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  unsigned Opcode = MI.getOpcode();

  auto setFIAndReturnSrcReg = [&](const MachineOperand &FIOp,
                                  const MachineOperand &RegOp) -> unsigned {
    if (FIOp.isFI() && RegOp.isReg()) {
      FrameIndex = FIOp.getIndex();
      return RegOp.getReg().id();
    }

    return 0;
  };

  switch (Opcode) {
  case STM8::LDispd8a:
  case STM8::PseudoLDispd8b:
  case STM8::GenLDWispd8r:
  case STM8::PseudoLDWispd8z:
    return setFIAndReturnSrcReg(MI.getOperand(1), MI.getOperand(2));
  default:
    break;
  }

  return 0;
}

void STM8InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, MCRegister DestReg,
                                MCRegister SrcReg, bool KillSrc) const {
  unsigned Opcode = 0;

  struct CopyRegInstr {
    const TargetRegisterClass &DstRC;
    const TargetRegisterClass &SrcRC;
    unsigned Opcode;
  };

  static const CopyRegInstr Instructions[] = {
      // Copy to A
      {STM8::AREGRegClass, STM8::XLREGRegClass, STM8::LDaxl},
      {STM8::AREGRegClass, STM8::XHREGRegClass, STM8::LDaxh},
      {STM8::AREGRegClass, STM8::YLREGRegClass, STM8::LDayl},
      {STM8::AREGRegClass, STM8::YHREGRegClass, STM8::LDayh},
      {STM8::AREGRegClass, STM8::BREGRegClass, STM8::PseudoLDab},
      // Copy to XL
      {STM8::XLREGRegClass, STM8::AREGRegClass, STM8::LDxla},
      // Copy to XH
      {STM8::XHREGRegClass, STM8::AREGRegClass, STM8::LDxha},
      // Copy to YL
      {STM8::YLREGRegClass, STM8::AREGRegClass, STM8::LDyla},
      // Copy to YH
      {STM8::YHREGRegClass, STM8::AREGRegClass, STM8::LDyha},
      // Copy to B
      {STM8::BREGRegClass, STM8::AREGRegClass, STM8::PseudoLDba},
      // Copy to X
      {STM8::XREGRegClass, STM8::YREGRegClass, STM8::LDWxy},
      // Copy to Y
      {STM8::YREGRegClass, STM8::XREGRegClass, STM8::LDWyx},
      // Copy to/from Z
      {STM8::ZREGRegClass, STM8::XYRegClass, STM8::GenLDWzr},
      {STM8::XYRegClass, STM8::ZREGRegClass, STM8::GenLDWrz},
  };

  for (const auto &CRI : Instructions) {
    if (CRI.SrcRC.contains(SrcReg) && CRI.DstRC.contains(DestReg)) {
      Opcode = CRI.Opcode;
      break;
    }
  }

  if (0 == Opcode) {
    if (copyBetweenBAndIdxSubreg(MBB, MI, DL, DestReg, SrcReg)) {
      return;
    }

    llvm_unreachable("Unsupported copy between physical registers");
  }

  LLVM_DEBUG(dbgs() << "* Inserting phys reg copy: " << getName(Opcode)
                    << "\n");
  BuildMI(MBB, MI, DL, get(Opcode), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
}

bool STM8InstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();
  const DebugLoc &DL = MI.getDebugLoc();
  unsigned Opc = MI.getOpcode();

  auto ExpandFusedJump = [&](unsigned TnzCpOpcode) {
    assert(MI.getNumExplicitOperands() > 2);
    unsigned CCOpNum = MI.getNumExplicitOperands() - 2;
    unsigned TargetOpNum = MI.getNumExplicitOperands() - 1;

    const MCInstrDesc &JrMCID = getJrCond(MI.getOperand(CCOpNum).getImm());

    LLVM_DEBUG(dbgs() << " * Replacing " << getName(MI.getOpcode()) << " with "
                      << getName(TnzCpOpcode) << " + "
                      << getName(JrMCID.getOpcode()) << "\n");

    MachineInstrBuilder Builder = BuildMI(MBB, MI, DL, get(TnzCpOpcode));
    for (unsigned OpNum = 0; OpNum < CCOpNum; ++OpNum) {
      Builder.add(MI.getOperand(OpNum));
    }

    BuildMI(MBB, MI, DL, JrMCID).add(MI.getOperand(TargetOpNum));

    MI.eraseFromParent();

    return true;
  };

  switch (Opc) {
  default:
    break;

  case STM8::TNZr_JR:
    return ExpandFusedJump(STM8::GenTNZr);
  case STM8::TNZispd8_JR:
    return ExpandFusedJump(STM8::TNZispd8);
  case STM8::TNZWr_JR:
    return ExpandFusedJump(STM8::GenTNZWr);
  case STM8::CPai8_JR:
    return ExpandFusedJump(STM8::CPai8);
  case STM8::CPab_JR:
    return ExpandFusedJump(STM8::PseudoCPab);
  case STM8::CPaispd8_JR:
    return ExpandFusedJump(STM8::CPaispd8);
  case STM8::CPWri16_JR:
    return ExpandFusedJump(STM8::GenCPWri16);
  case STM8::CPWxz_JR:
    return ExpandFusedJump(STM8::PseudoCPWxz);
  case STM8::CPWxispd8_JR:
    return ExpandFusedJump(STM8::CPWxispd8);
  }

  return false;
}

namespace {

struct BranchInfo {
  unsigned Opcode = 0;
  unsigned RevOpcode = 0;
  STM8::CondCode Cond = STM8::COND_OTHER;
  STM8::CondCode RevCond = STM8::COND_OTHER;

  BranchInfo getReversed() const { return {RevOpcode, Opcode, RevCond, Cond}; }
};

constexpr BranchInfo BranchInfos[] = {
    {STM8::JREQ, STM8::JRNE, STM8::COND_EQ, STM8::COND_NE},
    {STM8::JRSLT, STM8::JRSGE, STM8::COND_SLT, STM8::COND_SGE},
    {STM8::JRSLE, STM8::JRSGT, STM8::COND_SLE, STM8::COND_SGT},
    {STM8::JRC, STM8::JRNC, STM8::COND_ULT, STM8::COND_UGE},
    {STM8::JRULE, STM8::JRUGT, STM8::COND_ULE, STM8::COND_UGT},
    {STM8::JRMI, STM8::JRPL, STM8::COND_MI, STM8::COND_PL},
};

std::optional<BranchInfo> getBranchInfoForCC(unsigned CC) {
  for (const auto &BI : BranchInfos) {
    if (BI.Cond == CC) {
      return BI;
    } else if (BI.RevCond == CC) {
      return BI.getReversed();
    }
  }

  return std::nullopt;
}

std::optional<BranchInfo> getBranchInfoForOpcode(unsigned Opcode) {
  for (const auto &BI : BranchInfos) {
    if (BI.Opcode == Opcode) {
      return BI;
    } else if (BI.RevOpcode == Opcode) {
      return BI.getReversed();
    }
  }

  return std::nullopt;
}

struct BrInstrPair {
  MachineInstr *CondBr = nullptr;
  MachineInstr *FinalBr = nullptr;
};

BrInstrPair getTerminatingBranch(MachineBasicBlock &MBB) {
  auto LastMIIter = MBB.getLastNonDebugInstr();
  if ((LastMIIter == MBB.end()) || !LastMIIter->isBranch()) {
    return {};
  }

  BrInstrPair Result;
  if (LastMIIter->isConditionalBranch()) {
    Result.CondBr = &*LastMIIter;
    return Result;
  }

  Result.FinalBr = &*LastMIIter;
  if (LastMIIter != MBB.begin()) {
    auto PrevMIIter = std::prev(LastMIIter);
    if (PrevMIIter->isConditionalBranch()) {
      Result.CondBr = &*PrevMIIter;
    }
  }

  return Result;
}

} // namespace

const MCInstrDesc &STM8InstrInfo::getJrCond(unsigned CC) const {
  auto BI = getBranchInfoForCC(CC);
  if (BI.has_value()) {
    return get(BI->Opcode);
  }

  llvm_unreachable("Unexpected condition code");
}

STM8::CondCode STM8InstrInfo::getCodeCodeFromJrOpcode(unsigned Opcode) const {
  auto BI = getBranchInfoForOpcode(Opcode);
  if (BI.has_value()) {
    return BI->Cond;
  }

  return STM8::COND_OTHER;
}

bool STM8InstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool AllowModify) const {
  TBB = FBB = nullptr;
  Cond.clear();

  // TODO TODO TODO
  // This doesn't work if there's a conditional jump in a bundle!
  // For example: TNZ + JRNE
  BrInstrPair TermBr = getTerminatingBranch(MBB);

  if (!TermBr.CondBr && !TermBr.FinalBr) {
    // No branches, it's either fallthrough or return
    auto Iter = MBB.getLastNonDebugInstr();
    return (Iter != MBB.end()) ? Iter->isReturn() : false;
  }

  if (!TermBr.CondBr && TermBr.FinalBr) {
    // Unconditional branch

    MachineOperand &Op = TermBr.FinalBr->getOperand(0);
    if (!Op.isMBB()) {
      return true;
    }

    TBB = Op.getMBB();
    return false;
  }

  auto CondInfo = getBranchInfoForOpcode(TermBr.CondBr->getOpcode());
  MachineOperand &CondBrTargetOp = TermBr.CondBr->getOperand(0);
  if (!CondInfo.has_value() || !CondBrTargetOp.isMBB()) {
    return true;
  }

  if (!TermBr.FinalBr) {
    // Conditional branch followed by fallthrough

    TBB = CondBrTargetOp.getMBB();
    Cond.push_back(MachineOperand::CreateImm(CondInfo->Cond));
    return false;
  }

  // Conditional branch followed by unconditional branch

  MachineOperand &Op = TermBr.FinalBr->getOperand(0);
  if (!Op.isMBB()) {
    return true;
  }

  TBB = CondBrTargetOp.getMBB();
  FBB = Op.getMBB();
  Cond.push_back(MachineOperand::CreateImm(CondInfo->Cond));

  return false;
}

bool STM8InstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  if (Cond.empty()) {
    return true;
  }

  assert((Cond.size() == 1) && (Cond[0].isImm()));

  auto BI = getBranchInfoForCC(Cond[0].getImm());
  if (BI.has_value()) {
    Cond[0].setImm(BI->RevCond);
    return false;
  }

  return true;
}

unsigned STM8InstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {
  BrInstrPair TermBr = getTerminatingBranch(MBB);
  unsigned NumRemoved = 0;
  unsigned Bytes = 0;

  if (TermBr.CondBr) {
    Bytes += get(TermBr.CondBr->getOpcode()).getSize();
    TermBr.CondBr->removeFromParent();
    ++NumRemoved;
  }

  if (TermBr.FinalBr) {
    Bytes += get(TermBr.FinalBr->getOpcode()).getSize();
    TermBr.FinalBr->removeFromParent();
    ++NumRemoved;
  }

  if (BytesRemoved) {
    *BytesRemoved = Bytes;
  }

  return NumRemoved;
}

unsigned STM8InstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  unsigned NumAdded = 0;
  unsigned Bytes = 0;

  // Find iterator before which to insert new instructions
  auto InsertPos = MBB.getLastNonDebugInstr();
  if (InsertPos != MBB.end()) {
    InsertPos = std::next(InsertPos);
  }

  if (TBB && !FBB && Cond.empty()) {
    // Unconditional branch to TBB

    BuildMI(MBB, InsertPos, DL, get(STM8::JRA)).addMBB(TBB);
    NumAdded += 1;
    Bytes += get(STM8::JRA).getSize();
  } else if (TBB && !FBB && !Cond.empty()) {
    // Conditional branch to TBB followed by fallthrough

    auto BI = getBranchInfoForCC(Cond[0].getImm());
    assert(BI.has_value());
    const MCInstrDesc &MCID = get(BI->Opcode);
    BuildMI(MBB, InsertPos, DL, MCID).addMBB(TBB);
    NumAdded += 1;
    Bytes += MCID.getSize();
  } else if (TBB && FBB) {
    // Conditional branch to TBB followed by unconditional branch to FBB

    auto BI = getBranchInfoForCC(Cond[0].getImm());
    assert(BI.has_value());
    BuildMI(MBB, InsertPos, DL, get(BI->Opcode)).addMBB(TBB);
    BuildMI(MBB, InsertPos, DL, get(STM8::JRA)).addMBB(FBB);
    NumAdded += 2;
    Bytes += get(BI->Opcode).getSize() + get(STM8::JRA).getSize();
  } else if (!TBB && !FBB) {
    // No branches, just fallthrough
  } else {
    llvm_unreachable("Unexpected arguments passed to insertBranch()");
  }

  if (BytesAdded) {
    *BytesAdded = Bytes;
  }

  return NumAdded;
}

std::optional<DestSourcePair>
STM8InstrInfo::isCopyInstrImpl(const MachineInstr &MI) const {
  unsigned Opcode = MI.getOpcode();
  switch (Opcode) {
  case STM8::PseudoLDab:
  case STM8::PseudoLDba:
  case STM8::GenLDWrz:
  case STM8::GenLDWzr: {
    const MachineOperand &Dst = MI.getOperand(0);
    const MachineOperand &Src = MI.getOperand(1);
    return DestSourcePair(Dst, Src);
  }
  default:
    break;
  }

  return std::nullopt;
}

MachineInstr *STM8InstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr &MI, ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt, int FrameIndex, LiveIntervals *LIS,
    VirtRegMap *VRM) const {

  if (Ops.size() != 1) {
    return nullptr;
  }

  unsigned Opcode = MI.getOpcode();
  LLVM_DEBUG(dbgs() << "Trying to fold FI #" << FrameIndex << " as load into "
                    << getName(Opcode) << "\n");

  auto foldIntoUnaryInst = [&](unsigned NewOpcode) -> MachineInstr * {
    if (Ops[0] != 0) {
      return nullptr;
    }

    return BuildMI(*MI.getParent(), InsertPt, MI.getDebugLoc(), get(NewOpcode))
        .addFrameIndex(FrameIndex)
        .addImm(0);
  };

  MachineInstr *NewMI = nullptr;
  switch (Opcode) {
  default:
    return nullptr;
  case STM8::SLLa:
    NewMI = foldIntoUnaryInst(STM8::SLLispd8);
    break;
  case STM8::SRLa:
    NewMI = foldIntoUnaryInst(STM8::SRLispd8);
    break;
  case STM8::SRAa:
    NewMI = foldIntoUnaryInst(STM8::SRAispd8);
    break;
  case STM8::INCa:
    NewMI = foldIntoUnaryInst(STM8::INCispd8);
    break;
  case STM8::DECa:
    NewMI = foldIntoUnaryInst(STM8::DECispd8);
    break;
  case STM8::TNZa:
    NewMI = foldIntoUnaryInst(STM8::TNZispd8);
    break;
  case STM8::NEGa:
    NewMI = foldIntoUnaryInst(STM8::NEGispd8);
    break;
  }

  if (!NewMI) {
    return nullptr;
  }

  LLVM_DEBUG(dbgs() << " * Replacing " << getName(Opcode) << " with ");
  LLVM_DEBUG(NewMI->print(dbgs()));

  return NewMI;
}

bool STM8InstrInfo::canReMatImplicitRegisterDef(const MachineInstr & /*MI*/,
                                                const Register &Reg) const {
  // LD/LDW instructions, which are used for spills and reloads, set CC.n and
  // CC.z. This means that we have to bundle defs and uses of these registers if
  // we want to use them (example: TNZ instruction followed by a conditional
  // jump).
  //
  // Without rematerialization, register reload would change CC.n and CC.z
  // either way so we can allow for rematerialization of such instructions.
  return (Reg == STM8::CCn) || (Reg == STM8::CCz);
}

bool STM8InstrInfo::copyBetweenBAndIdxSubreg(MachineBasicBlock &MBB,
                                             MachineBasicBlock::iterator MI,
                                             const DebugLoc &DL,
                                             MCRegister DestReg,
                                             MCRegister SrcReg) const {
  bool BToIdxLo = false, BToIdxHi = false, IdxLoToB = false, IdxHiToB = false;
  if (STM8::BREGRegClass.contains(SrcReg)) {
    BToIdxLo = isIdxLowSubreg(DestReg);
    BToIdxHi = isIdxHighSubreg(DestReg);
  } else if (STM8::BREGRegClass.contains(DestReg)) {
    IdxLoToB = isIdxLowSubreg(SrcReg);
    IdxHiToB = isIdxHighSubreg(SrcReg);
  } else {
    return false;
  }

  bool CopyToB = (IdxLoToB || IdxHiToB);
  bool CopyFromB = (BToIdxLo || BToIdxHi);

  if (!CopyToB && !CopyFromB) {
    return false;
  }

  unsigned IdxReg =
      getIdxRegFromSubreg((BToIdxHi || BToIdxLo) ? DestReg : SrcReg);
  MCRegister IdxSubreg = CopyToB ? SrcReg : DestReg;

  unsigned SwapOpcode = (IdxReg == STM8::X) ? STM8::SWAPWx : STM8::SWAPWy;
  unsigned ExgOpcode = (IdxReg == STM8::X) ? STM8::EXGaxl : STM8::EXGayl;

  if (BToIdxHi || IdxHiToB) {
    // Swap high and low parts of the index register
    BuildMI(MBB, MI, DL, get(SwapOpcode), IdxReg).addReg(IdxReg);
  }

  // Exchange XL/YL with A
  BuildMI(MBB, MI, DL, get(ExgOpcode))
      .addDef(STM8::A)
      .addDef(IdxSubreg)
      .addReg(STM8::A)
      .addReg(IdxSubreg);

  if (CopyFromB) {
    // Copy B to A
    BuildMI(MBB, MI, DL, get(STM8::PseudoLDab), STM8::A).addReg(SrcReg);
  } else {
    // Copy A to B
    BuildMI(MBB, MI, DL, get(STM8::PseudoLDba), DestReg).addReg(STM8::A);
  }

  // Restore A and XL/YL
  BuildMI(MBB, MI, DL, get(ExgOpcode))
      .addDef(STM8::A)
      .addDef(IdxSubreg)
      .addReg(STM8::A)
      .addReg(IdxSubreg);

  if (BToIdxHi || IdxHiToB) {
    // Restore low and high parts of the index register
    BuildMI(MBB, MI, DL, get(SwapOpcode), IdxReg).addReg(IdxReg);
  }

  return true;
}

bool STM8InstrInfo::isIdxLowSubreg(unsigned Reg) const {
  return STM8::XLREGRegClass.contains(Reg) || STM8::YLREGRegClass.contains(Reg);
}

bool STM8InstrInfo::isIdxHighSubreg(unsigned Reg) const {
  return STM8::XHREGRegClass.contains(Reg) || STM8::YHREGRegClass.contains(Reg);
}

unsigned STM8InstrInfo::getIdxRegFromSubreg(unsigned Subreg) const {
  if (STM8::XLREGRegClass.contains(Subreg) ||
      STM8::XLREGRegClass.contains(Subreg)) {
    return STM8::X;
  }

  if (STM8::YLREGRegClass.contains(Subreg) ||
      STM8::YLREGRegClass.contains(Subreg)) {
    return STM8::Y;
  }

  return 0;
}

} // namespace llvm
