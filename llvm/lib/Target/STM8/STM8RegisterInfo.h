#ifndef LLVM_TARGET_STM8_REGISTER_INFO_H_INCLUDED
#define LLVM_TARGET_STM8_REGISTER_INFO_H_INCLUDED

#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "STM8GenRegisterInfo.inc"

namespace llvm {

class STM8RegisterInfo : public STM8GenRegisterInfo {
public:
  STM8RegisterInfo();

  const uint16_t *
  getCalleeSavedRegs(const MachineFunction *MF = nullptr) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  /** TODO fill missing docs
   * @param MI Iterator pointing to an instruction which has Frame Index as an operand.
   * @param SPAdj Stack pointer adjustment. @todo What is this?
   * @param FIOperandNum Index of the operand which is the Frame Index.
   * @param RS @todo What is this?
   * @return true 
   * @return false 
   */
  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // namespace llvm

#endif // LLVM_TARGET_STM8_REGISTER_INFO_H_INCLUDED
