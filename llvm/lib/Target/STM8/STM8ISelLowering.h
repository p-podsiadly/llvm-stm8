#ifndef LLVM_STM8_ISEL_LOWERING_H_INCLUDED
#define LLVM_STM8_ISEL_LOWERING_H_INCLUDED

#include "STM8InstrInfo.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class STM8TargetMachine;
class STM8Subtarget;

namespace STM8 {

enum {
  // Start the numbering where the builtin ops and target ops leave off.
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  /// Wrapper for TargetExternalSymbol, TargetGlobalSymbol and TargetJumpTable.
  /// Apparently, VReg for these nodes is not created. Putting them inside
  /// WRAPPER is necessary to make it work.
  WRAPPER,
  CALL,
  RET_GLUE,
  IRET_GLUE,
  /// @brief Fused TNZ-conditional jump operation.
  ///
  /// Operands: 1) Value to test, 2) CondCode constant (i8), 3) jump target.
  /// Results: output glue.
  TNZ_JUMP,
  /// @brief  Fused compare-conditional jump operation.
  ///
  /// Operands: 1) LHS of comparison, 2) RHS of comparison, 3) CondCode constant
  /// (i8), 4)jump target.
  /// Results: output glue.
  COMPARE_JUMP,
  PUSH,
  POP,
  /// @brief Carry-setting addition.
  ///
  /// This node represents an integer addition which sets carry bit in the CC
  /// register.
  ///
  /// Operands: two integers of the same size.
  /// Results: 1) sum of the operands, 2) Carry flag of the CC register (i1).
  ADD,
  /// @brief Carry-setting addition with incoming carry value.
  ///
  /// Operands: two integers of the same size and Carry flag of the CC register
  /// (i1).
  /// Results: 1) sum of the operands and carry, 2) Carry flag of the CC
  /// register (i1).
  ADD_CARRY,
  /// @brief Select between two values based on the input value.
  ///
  /// Operands: 1) input value, 2) true value , 3) false value, 4) CondCode
  ///           constant (i8).
  /// Result: True or false value depending on the result of test (TNZ) of the
  ///         input value.
  TNZ_SELECT_CC,
  /// @brief Select between two values based on the result of comparison.
  ///
  /// Operands: 1) LHS of comparison, 2) RHS of comparison, 3) true value,
  ///           4) false value, 5) CondCode constant (i8).
  /// Result: True or false value, depending on the result of comparison.
  COMPARE_SELECT_CC,
  /// @brief Set low or high half of a 16-bit value.
  ///
  /// LLVM treats target-independent EXTRACT_ELEMENT node specially - it will be
  /// expanded even when it is marked as legal. This means that it cannot be
  /// used for selection patterns. The workaround is to define SET_BYTE/GET_BYTE
  /// nodes and set custom legalization action for EXTRACT_ELEMENT. This way
  /// selection patterns don't have to deal with "generic" expansions (shifts
  /// and truncations).
  ///
  /// Operands:
  /// 1) 16-bit value of which the high part will be preserved,
  /// 2) Half of the value: 0 for low, 1 for high (8-bit),
  /// 3) 8-bit value.
  ///
  /// Result: 16-bit value composed of high part of the first operand and the
  /// second operand.
  SET_BYTE,
  /// @brief Read low or high part of a 16-bit value.
  ///
  /// Operands:
  /// 1) 16-bit value to read from,
  /// 2) Half of the value: 0 for low, 1 for high (8-bit).
  ///
  /// Result: 8-bit value of the specified half of the input value.
  GET_BYTE,
  /// Swap low and high bytes of a 16 bit value.
  ///
  /// Operand: 16 bit value
  /// Result: 16 bit value with swap low and high bytes
  SWAP_BYTES,
  /// Shift left by one bit.
  SHL,
  /// Logical shift right by one bit.
  SHR,
  /// Arithmetic shift right by one bit.
  ASHR,
  /// Shift left by a variable number of bits.
  ///
  /// STM8 supports only single-bit shifts, variable shifts need to be expanded
  /// into a loop.
  /// A loop requires a new basic block, so we need to:
  /// 1) emit a VAR_SHIFT node,
  /// 2) select it as a pseudo instruction and
  /// 3) use EmitInstrWithCustomInserter() to create a new basic block with the
  ///    body of the loop.
  ///
  /// Operands: 1) value to shift (i8 or i16), 2) shift amount (same type as
  /// the first operand).
  /// Result: shifted value, same type as the first operand.
  VAR_SHL,
  /// Logical shift right by a variable number of bits. See VAR_SHL.
  VAR_SHR,
  /// Arithmetic shift right by a variable number of bits. See VAR_SHL.
  VAR_ASHR,
  /// Unsigned integer division with reminder.
  ///
  /// Operands: 1) LHS value (i16), 2) RHS value (i8 or i16)
  /// Results: 1) Quotient (i16), 2) Reminder (i8 or i16)
  UDIVREM,
};

} // namespace STM8

class STM8TargetLowering : public TargetLowering {
public:
  explicit STM8TargetLowering(const STM8TargetMachine &TM,
                              const STM8Subtarget &STI);

  const char *getTargetNodeName(unsigned Opcode) const override;

  EVT getTypeForExtReturn(LLVMContext &Context, EVT VT,
                          ISD::NodeType /*ExtendKind*/) const override;

  MVT getScalarShiftAmountTy(const DataLayout &, EVT LHSTy) const override;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *MBB) const override;

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;

  SDValue LowerCall(CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

private:
  const STM8Subtarget &Subtarget;

  SDValue LowerCallResult(SDValue Chain, SDValue Glue, CallingConv::ID CC,
                          bool IsVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          const SDLoc &DL, SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;

  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerSignExt(SDValue Op, SelectionDAG &DAG) const;

  /// Lowers 16-bit AND/OR/XOR to a pair of 8-bit operations.
  SDValue LowerWordLogicalOp(SDValue Op, SelectionDAG &DAG) const;

  /// @brief Lowers shift instructions.
  ///
  /// STM8 supports only shifts by one position. Variable shifts need to be
  /// lowered to a loop.
  SDValue LowerShiftOp(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;

  /// @brief Lowers UDIV/UDIVREM/UREM to DIV/DIVW.
  SDValue LowerUDivURem(SDValue Op, SelectionDAG &DAG) const;

  MachineBasicBlock *emitSelect(MachineInstr &MI, MachineBasicBlock *MBB) const;
  MachineBasicBlock *emitVarShift(MachineInstr &MI,
                                  MachineBasicBlock *MBB) const;
  MachineBasicBlock *emitVarShiftParts(MachineInstr &MI,
                                       MachineBasicBlock *MBB) const;

  /// @brief Creates STM8-specific condition code node, ie constant with a value
  /// from STM8::CondCode.
  unsigned getCondCodeConstant(ISD::CondCode CC, unsigned *OutTnzCC) const;

  SDValue getFusedCompareJumpNode(SelectionDAG &DAG, const SDLoc &DL,
                                  const SDValue &InChain, ISD::CondCode CC,
                                  const SDValue &LHS, const SDValue &RHS,
                                  const SDValue &JumpTarget) const;

  /// @brief Convert the value to i8 TargetConstant or return the value
  /// unchanged.
  ///
  /// Turning Constant into TargetConstant prevents further attempts to select.
  /// This method helps enforcing correct selection.
  SDValue tryGetTargetConst(SelectionDAG &DAG, const SDValue &Val) const;

  /// @brief Create a new MBB and moves all instructions after the specified
  /// instruction to it.
  MachineBasicBlock *createTailMBB(MachineBasicBlock *MBB,
                                   MachineInstr &splitAfter) const;
};

} // namespace llvm

#endif // LLVM_STM8_ISEL_LOWERING_H_INCLUDED
