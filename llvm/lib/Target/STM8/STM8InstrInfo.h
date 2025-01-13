#ifndef LLVM_STM8_INSTR_INFO_H_INCLUDED
#define LLVM_STM8_INSTR_INFO_H_INCLUDED

#include <llvm/CodeGen/TargetInstrInfo.h>

#define GET_INSTRINFO_HEADER
#include "STM8GenInstrInfo.inc"

namespace llvm {

namespace STM8 {
enum CondCode {
  COND_OTHER = 0,
  COND_EQ = 1,
  COND_NE = 2,
  COND_SLT = 3,
  COND_SLE = 4,
  COND_SGT = 5,
  COND_SGE = 6,
  COND_ULT = 7,
  COND_ULE = 8,
  COND_UGT = 9,
  COND_UGE = 10,
  COND_MI = 11,
  COND_PL = 12,

  // Alias for clarity
  COND_CARRY = COND_ULT,
};
}

class STM8InstrInfo : public STM8GenInstrInfo {
public:
  STM8InstrInfo();

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, Register SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI,
                           Register VReg) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI, Register DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI,
                            Register VReg) const override;

  unsigned isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;
  unsigned isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;

  /// Return opcode of relative conditional jump corresponding to STM8::CondCode
  /// value.
  const MCInstrDesc &getJrCond(unsigned CC) const;
  STM8::CondCode getCodeCodeFromJrOpcode(unsigned Opcode) const;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

protected:
  std::optional<DestSourcePair>
  isCopyInstrImpl(const MachineInstr &MI) const override;

  MachineInstr *foldMemoryOperandImpl(MachineFunction &MF, MachineInstr &MI,
                                      ArrayRef<unsigned> Ops,
                                      MachineBasicBlock::iterator InsertPt,
                                      int FrameIndex,
                                      LiveIntervals *LIS = nullptr,
                                      VirtRegMap *VRM = nullptr) const override;

  bool canReMatImplicitRegisterDef(const MachineInstr &MI,
                                   const Register &Reg) const override;

private:
  bool copyBetweenBAndIdxSubreg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, MCRegister DestReg,
                                MCRegister SrcReg) const;

  bool isIdxLowSubreg(unsigned Reg) const;
  bool isIdxHighSubreg(unsigned Reg) const;
  unsigned getIdxRegFromSubreg(unsigned Subreg) const;
};

} // namespace llvm

#endif // LLVM_STM8_INSTR_INFO_H_INCLUDED
