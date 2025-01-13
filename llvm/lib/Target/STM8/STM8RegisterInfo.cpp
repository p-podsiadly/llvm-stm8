#include "STM8RegisterInfo.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"

#define GET_REGINFO_TARGET_DESC
#include "STM8GenRegisterInfo.inc"

namespace llvm {

STM8RegisterInfo::STM8RegisterInfo() : STM8GenRegisterInfo(0) {}

const uint16_t *
STM8RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  ((void)MF); // The same saved regs for all calling conventions
  return CSR_Normal_SaveList;
}

const uint32_t *
STM8RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const {
  // Same for all calling conventions
  ((void)MF);
  ((void)CC);
  return CSR_Normal_RegMask;
}

BitVector STM8RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Regs(getNumRegs());

  Regs.set(STM8::SP);
  return Regs;
}

bool STM8RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MIIter,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  MachineInstr &MI = *MIIter;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  const TargetFrameLowering *TFL = MF.getSubtarget().getFrameLowering();
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  int64_t Displacement = MI.getOperand(FIOperandNum + 1).getImm();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  int64_t StackSize = MFI.getStackSize();
  int64_t ObjOffset = MFI.getObjectOffset(FrameIndex);

  // Offset of the local area returned from TargetFrameLowering is negative as
  // the stack grows down. SP points to the first free slot below the stack, not
  // to the bottom of the stack - this is accounted for by "+1".
  int64_t StackOffset = StackSize + ObjOffset - TFL->getOffsetOfLocalArea() +
                        1 + Displacement + SPAdj;
  assert(StackOffset >= 0);

  // SP-relative addressing is possible only with 8-bit offsets. When the offset
  // is larger we temporarily change SP so that the actual memory access can be
  // done with 8-bit offset.
  //
  // Note: STM8FrameLowering::emitPrologue() emits a
  // warning when stack frame size is greater than 255 bytes so that such cases
  // don't go undetected.
  MachineBasicBlock::iterator RestoreMIIter = std::next(MIIter);
  int64_t SPOffset = StackOffset;
  while (SPOffset > 255) {
    int64_t Step = std::min(SPOffset, INT64_C(255));
    // Adjust SP before MI
    BuildMI(MBB, MIIter, MI.getDebugLoc(), TII->get(STM8::ADDWspi8), STM8::SP)
        .addReg(STM8::SP, RegState::Kill)
        .addImm(Step);
    // Restore SP after MI
    BuildMI(MBB, RestoreMIIter, MI.getDebugLoc(), TII->get(STM8::SUBspi8),
            STM8::SP)
        .addReg(STM8::SP, RegState::Kill)
        .addImm(Step);
    SPOffset -= Step;
  }

  // Change FrameIndex+Offset to SP+Offset.
  MI.getOperand(FIOperandNum).ChangeToRegister(STM8::SP, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(SPOffset);

  return false;
}

Register STM8RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  // STM8 does not have spare registers to use as FP.
  return STM8::SP;
}

} // namespace llvm
