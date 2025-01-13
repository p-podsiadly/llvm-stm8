#ifndef LLVM_STM8_FRAME_LOWERING_H_INCLUDED
#define LLVM_STM8_FRAME_LOWERING_H_INCLUDED

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class STM8FrameLowering : public TargetFrameLowering {
public:
  STM8FrameLowering();

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

private:
  MachineBasicBlock::iterator
  adjustCallStackBeforeCall(MachineFunction &MF, MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI) const;
  MachineBasicBlock::iterator
  adjustCallStackAfterCall(MachineFunction &MF, MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI) const;
};

} // namespace llvm

#endif // LLVM_STM8_FRAME_LOWERING_H_INCLUDED
