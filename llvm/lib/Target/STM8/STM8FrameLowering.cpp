#include "STM8FrameLowering.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8InstrInfo.h"
#include "STM8MachineFunctionInfo.h"
#include "STM8Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/IR/DiagnosticInfo.h"

#define DEBUG_TYPE "stm8-frame-lowering"

namespace llvm {

STM8FrameLowering::STM8FrameLowering()
    : TargetFrameLowering(StackGrowsDown, Align(1), -2) {}

void STM8FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  // Iterator pointing to the first instruction in the body of the function.
  // There may be some frame setup instructions before it.
  MachineBasicBlock::iterator MIIter = MBB.begin();
  DebugLoc DL = (MIIter != MBB.end()) ? MIIter->getDebugLoc() : DebugLoc();

  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

  const MachineFrameInfo &FrameInfo = MF.getFrameInfo();
  uint64_t StackSize = FrameInfo.getStackSize();

  // The backend is able to handle frames larger than 255 bytes, but given
  // limited resources of STM8 MCUs it's better to avoid this. We report such
  // cases as a warning.
  if (!isUInt<8>(StackSize)) {
    Function &Func = MF.getFunction();
    LLVMContext &Ctx = Func.getContext();

    Ctx.diagnose(DiagnosticInfoResourceLimit(Func, "preferred stack frame size",
                                             StackSize, 255, DS_Warning));
  }

  // SP can be changed by at most 255 at a time. If stack is larger, SP has to
  // be decreased multiple times.
  while (StackSize > 0) {
    uint64_t AdjAmount = std::min(StackSize, UINT64_C(255));

    BuildMI(MBB, MIIter, DL, TII->get(STM8::SUBspi8), STM8::SP)
        .addReg(STM8::SP, RegState::Kill)
        .addImm(AdjAmount);

    StackSize -= AdjAmount;
  }
}

void STM8FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {

  // Iterator pointing to the terminating RET instruction.
  // It is used for inserting frame teardown instructions immediately before
  // returning.
  MachineBasicBlock::iterator MIIter = MBB.getLastNonDebugInstr();
  assert(MIIter->getDesc().isReturn() && "Expected a returning basic block");
  DebugLoc DL = (MIIter != MBB.end()) ? MIIter->getDebugLoc() : DebugLoc();

  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

  MachineFrameInfo &FrameInfo = MF.getFrameInfo();
  uint64_t StackSize = FrameInfo.getStackSize();

  // Like in emitPrologue(), SP can be adjusted by at most 255 at a time.
  while (StackSize > 0) {
    uint64_t AdjAmount = std::min(StackSize, UINT64_C(255));

    BuildMI(MBB, MIIter, DL, TII->get(STM8::ADDWspi8), STM8::SP)
        .addReg(STM8::SP, RegState::Kill)
        .addImm(AdjAmount);

    StackSize -= AdjAmount;
  }
}

bool STM8FrameLowering::hasFP(const MachineFunction &MF) const {
  // STM8 has too few registers to sacrifice one for the frame pointer
  ((void)MF);
  return false;
}

MachineBasicBlock::iterator STM8FrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {

  if (hasReservedCallFrame(MF)) {
    // For functions with reserved call frames stack area for calls is allocated
    // immediately upon entry. This eliminates the need to change SP and adjust
    // SP-relative offsets.
    return MBB.erase(MI);
  }

  unsigned Opc = MI->getOpcode();
  switch (Opc) {
  case STM8::ADJCALLSTACKDOWN:
    return adjustCallStackBeforeCall(MF, MBB, MI);
  case STM8::ADJCALLSTACKUP:
    return adjustCallStackAfterCall(MF, MBB, MI);
  default:
    break;
  }

  llvm_unreachable("Unexpected opcode!");
}

MachineBasicBlock::iterator STM8FrameLowering::adjustCallStackBeforeCall(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {

  // TODO convert LD/LDW to PUSH/PUSHW?

  const STM8Subtarget &STI = MF.getSubtarget<STM8Subtarget>();
  const STM8InstrInfo &TII = *STI.getInstrInfo();

  int64_t Amount = TII.getFrameSize(*MI);
  if (Amount != 0) {
    assert(isUInt<8>(Amount) && "Frame size cannot exceed range [0, 255]");

    BuildMI(MBB, MI, MI->getDebugLoc(), TII.get(STM8::SUBspi8), STM8::SP)
        .addReg(STM8::SP)
        .addImm(Amount);
  }

  return MBB.erase(MI);
}

MachineBasicBlock::iterator STM8FrameLowering::adjustCallStackAfterCall(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  const STM8Subtarget &STI = MF.getSubtarget<STM8Subtarget>();
  const STM8InstrInfo &TII = *STI.getInstrInfo();

  int64_t Amount = TII.getFrameSize(*MI);
  if (Amount != 0) {
    assert(isUInt<8>(Amount) && "Frame size cannot exceed range [0, 255]");

    BuildMI(MBB, MI, MI->getDebugLoc(), TII.get(STM8::ADDWspi8), STM8::SP)
        .addReg(STM8::SP)
        .addImm(Amount);
  }

  return MBB.erase(MI);
}

} // namespace llvm
