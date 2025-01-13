#include "STM8ISelLowering.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8CallingConv.h"
#include "STM8MachineFunctionInfo.h"
#include "STM8RegisterInfo.h"
#include "STM8TargetMachine.h"
#include <llvm/CodeGen/MachineFrameInfo.h>

#define DEBUG_TYPE "stm8-isel-lowering"

namespace llvm {

STM8TargetLowering::STM8TargetLowering(const STM8TargetMachine &TM,
                                       const STM8Subtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  addRegisterClass(MVT::i8, &STM8::AREGRegClass);
  addRegisterClass(MVT::i8, &STM8::BREGRegClass);
  addRegisterClass(MVT::i16, &STM8::XYRegClass);
  addRegisterClass(MVT::i16, &STM8::ZREGRegClass);

  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(STM8::SP);

  setBooleanContents(ZeroOrOneBooleanContent);

  setOperationAction(ISD::BR_CC, {MVT::i8, MVT::i16}, Custom);
  setOperationAction({ISD::BRCOND, ISD::BR_JT}, MVT::Other, Expand);
  setOperationAction(ISD::SELECT, {MVT::i8, MVT::i16}, Expand);
  setOperationAction(ISD::SELECT_CC, {MVT::i8, MVT::i16}, Custom);
  setOperationAction(ISD::SETCC, {MVT::i8, MVT::i16}, Expand);

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);
  setOperationAction(ISD::JumpTable, MVT::i16, Custom);

  setOperationAction({ISD::AND, ISD::OR, ISD::XOR}, MVT::i16, Custom);

  setOperationAction({ISD::SHL, ISD::SRA, ISD::SRL}, {MVT::i8, MVT::i16},
                     Custom);

  setOperationAction(ISD::MUL, MVT::i16, LibCall);
  setOperationAction({ISD::UMUL_LOHI, ISD::SMUL_LOHI, ISD::MULHU, ISD::MULHS},
                     MVT::i16, Expand);
  setOperationAction({ISD::UMUL_LOHI, ISD::MULHU}, MVT::i8, Legal);
  setOperationAction({ISD::SMUL_LOHI, ISD::MULHS}, MVT::i8, Expand);

  setOperationAction({ISD::UDIV, ISD::UREM, ISD::UDIVREM}, {MVT::i8, MVT::i16},
                     Custom);
  setOperationAction({ISD::SDIV, ISD::SREM, ISD::SDIVREM}, {MVT::i8, MVT::i16},
                     Expand);

  setOperationAction(
      {ISD::CTLZ, ISD::CTTZ, ISD::CTPOP, ISD::BITREVERSE, ISD::PARITY},
      {MVT::i8, MVT::i16}, Expand);

  setOperationAction(ISD::SIGN_EXTEND, MVT::i16, Custom);

  // Support for variadic functions
  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction({ISD::VAARG, ISD::VACOPY, ISD::VAEND}, MVT::Other, Expand);

  setLoadExtAction({ISD::EXTLOAD, ISD::SEXTLOAD, ISD::ZEXTLOAD}, MVT::i16,
                   MVT::i8, Expand);
  setTruncStoreAction(MVT::i16, MVT::i8, Expand);
}

const char *STM8TargetLowering::getTargetNodeName(unsigned Opcode) const {
#define OPCODE_TO_STR(OpCode)                                                  \
  case STM8::OpCode:                                                           \
    return #OpCode;

  switch (Opcode) {
    OPCODE_TO_STR(WRAPPER)
    OPCODE_TO_STR(CALL)
    OPCODE_TO_STR(RET_GLUE)
    OPCODE_TO_STR(IRET_GLUE)
    OPCODE_TO_STR(TNZ_JUMP)
    OPCODE_TO_STR(COMPARE_JUMP)
    OPCODE_TO_STR(PUSH)
    OPCODE_TO_STR(POP)
    OPCODE_TO_STR(ADD)
    OPCODE_TO_STR(ADD_CARRY)
    OPCODE_TO_STR(TNZ_SELECT_CC)
    OPCODE_TO_STR(COMPARE_SELECT_CC)
    OPCODE_TO_STR(SET_BYTE)
    OPCODE_TO_STR(GET_BYTE)
    OPCODE_TO_STR(SWAP_BYTES)
    OPCODE_TO_STR(SHL)
    OPCODE_TO_STR(SHR)
    OPCODE_TO_STR(ASHR)
    OPCODE_TO_STR(VAR_SHL)
    OPCODE_TO_STR(VAR_SHR)
    OPCODE_TO_STR(VAR_ASHR)
    OPCODE_TO_STR(UDIVREM)
  default:
    break;
  }

#undef OPCODE_TO_STR

  return nullptr;
}

EVT STM8TargetLowering::getTypeForExtReturn(
    LLVMContext & /*Context*/, EVT VT, ISD::NodeType /*ExtendKind*/) const {
  // Always use 8bit/16bit registers, as specified by the calling convention.
  // The default implementation returns i16 for sign-extended i8 values, which
  // violates the calling convention rules.
  EVT MinVT = getRegisterType(MVT::i8);
  return VT.bitsLT(MinVT) ? MinVT : VT;
}

MVT STM8TargetLowering::getScalarShiftAmountTy(const DataLayout &, EVT) const {
  // STM8 natively supports only one-bit shifts, so the RHS type is arbitrary.
  return MVT::i8;
}

MachineBasicBlock *
STM8TargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                MachineBasicBlock *MBB) const {
  unsigned Opcode = MI.getOpcode();

  switch (Opcode) {
  case STM8::TnzSelect8r:
  case STM8::TnzSelect16r:
  case STM8::CompareSelect8ai8:
  case STM8::CompareSelect8ab:
  case STM8::CompareSelect16ri16:
  case STM8::CompareSelect16xz:
    return emitSelect(MI, MBB);
  case STM8::PseudoVarSLL:
  case STM8::PseudoVarSRL:
  case STM8::PseudoVarSRA:
  case STM8::PseudoVarSLLW:
  case STM8::PseudoVarSRLW:
  case STM8::PseudoVarSRAW:
    return emitVarShift(MI, MBB);
  case STM8::PseudoVarSLLWParts:
  case STM8::PseudoVarSRLWParts:
    return emitVarShiftParts(MI, MBB);
  default:
    break;
  }

  llvm_unreachable("Custom instruction inserter: unexpected MachineInstr");
  return nullptr;
}

SDValue STM8TargetLowering::LowerOperation(SDValue Op,
                                           SelectionDAG &DAG) const {
  unsigned OpCode = Op.getOpcode();

  switch (OpCode) {
  case ISD::BR_CC:
    return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::JumpTable:
    return LowerJumpTable(Op, DAG);
  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  case ISD::SIGN_EXTEND:
    return LowerSignExt(Op, DAG);
  case ISD::AND:
  case ISD::OR:
  case ISD::XOR:
    return LowerWordLogicalOp(Op, DAG);
  case ISD::SHL:
  case ISD::SRA:
  case ISD::SRL:
    return LowerShiftOp(Op, DAG);
  case ISD::UDIV:
  case ISD::UDIVREM:
  case ISD::UREM:
    return LowerUDivURem(Op, DAG);
  default:
    break;
  }

  return SDValue();
}

SDValue STM8TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DebugLoc,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  // Locations (offset on the stack or a register) assigned to each argument.
  // It's safe to assume that most functions have at most 8 arguments.
  // For functions with larger numbers of arguments heap allocation will be
  // required.
  SmallVector<CCValAssign, 8> ArgLocs;

  // CCInfo keeps track of locations assigned to arguments. Assignments are made
  // according to rules of the calling convention.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCAssignFn *AssignFn = STM8::getCCAssignFn(CallConv);
  assert(AssignFn && "Unknown calling convention, could not get CCAssignFn!");

  CCInfo.AnalyzeFormalArguments(Ins, AssignFn);

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  for (unsigned ArgIdx = 0; ArgIdx < ArgLocs.size(); ++ArgIdx) {
    CCValAssign &VA = ArgLocs[ArgIdx];

    if (VA.isRegLoc()) {
      const TargetRegisterClass *RC = nullptr;
      switch (VA.getLocReg().id()) {
      case STM8::A:
        RC = &STM8::AREGRegClass;
        break;
      case STM8::X:
        RC = &STM8::XREGRegClass;
        break;
      case STM8::Y:
        RC = &STM8::YREGRegClass;
        break;
      default:
        llvm_unreachable("Unexpected argument register");
      }

      Register Reg = MF.addLiveIn(VA.getLocReg(), RC);
      SDValue ArgValue =
          DAG.getCopyFromReg(Chain, DebugLoc, Reg, VA.getValVT());
      InVals.push_back(ArgValue);
    } else if (VA.isMemLoc()) {
      EVT ArgVT = VA.getLocVT();
      unsigned BitSize = ArgVT.getSizeInBits();

      int FrameIdx =
          MFI.CreateFixedObject(BitSize / 8, VA.getLocMemOffset(), true);

      EVT PtrType = MVT::getIntegerVT(16);
      SDValue FIN = DAG.getFrameIndex(FrameIdx, PtrType);

      InVals.push_back(
          DAG.getLoad(ArgVT, DebugLoc, Chain, FIN,
                      MachinePointerInfo::getFixedStack(MF, FrameIdx)));
    } else {
      llvm_unreachable("Unexpected location of an assigned argument!");
    }
  }

  if (isVarArg) {
    // If this is a variadic function, we need to allocate a stack slot for the
    // pointer to the first variadic argument. This will be used for by
    // va_start() to initialize the list.
    auto *FuncInfo = MF.getInfo<STM8MachineFunctionInfo>();

    int FI = MFI.CreateFixedObject(
        /*Size=*/2, /*SPOffset=*/CCInfo.getStackSize(), /*IsImmutable=*/true);

    FuncInfo->SetVaArgsFrameIndex(FI);
  }

  return Chain;
}

SDValue
STM8TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &dl, SelectionDAG &DAG) const {

  SmallVector<CCValAssign, 4> RVLocs;

  auto *MFInfo = DAG.getMachineFunction().getInfo<STM8MachineFunctionInfo>();
  if (MFInfo->IsInterruptHandler()) {
    if (Outs.size() != 0) {
      llvm_unreachable("Interrupt handlers cannot have return values");
    }

    return DAG.getNode(STM8::IRET_GLUE, dl, MVT::Other, Chain);
  }

  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, STM8::getRetCCAssignFn(CallConv));

  SmallVector<SDValue, 4> OutChains;
  OutChains.push_back(Chain);

  SmallVector<SDValue, 4> RetOps(1, Chain);
  for (unsigned i = 0; i < RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];

    if (VA.isRegLoc()) {
      OutChains.push_back(
          DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i]));

      RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
    } else {
      llvm_unreachable(
          "TODO for now only returning through registers is supported");
    }
  }

  // Update the chain. Output registers can be assigned in any order. Reordering
  // assignments might help with better scheduling (note: two register results
  // are used for example for returning i32 values).
  RetOps[0] = DAG.getTokenFactor(dl, OutChains);

  // TODO support for returning from far functions
  return DAG.getNode(STM8::RET_GLUE, dl, MVT::Other, RetOps);
}

SDValue STM8TargetLowering::LowerCall(CallLoweringInfo &CLI,
                                      SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = CLI.DAG.getMachineFunction();

  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;

  SmallVector<CCValAssign, 8> ArgLocs;
  SmallVector<SDValue, 8> MemOpChains;

  // TODO: implement tail calls
  CLI.IsTailCall = false;

  CCState CCInfo(CLI.CallConv, CLI.IsVarArg, MF, ArgLocs,
                 *CLI.DAG.getContext());

  CCInfo.AnalyzeCallOperands(CLI.Outs, STM8::getCCAssignFn(CLI.CallConv));

  // Get a count of how many bytes are to be pushed on the stack.
  const unsigned NumBytes = CCInfo.getStackSize();
  Chain = CLI.DAG.getCALLSEQ_START(Chain, NumBytes, 0, CLI.DL);

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;

  for (unsigned ArgIdx = 0; ArgIdx < ArgLocs.size(); ++ArgIdx) {
    CCValAssign &VA = ArgLocs[ArgIdx];

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), CLI.OutVals[ArgIdx]));
    } else if (VA.isMemLoc()) {
      // SP points to one stack slot further so add one to adjust it.
      int64_t Offset = VA.getLocMemOffset() + 1;

      if (isUInt<16>(Offset)) {
        SDValue PtrOff = CLI.DAG.getNode(
            ISD::ADD, CLI.DL, MVT::i16, CLI.DAG.getRegister(STM8::SP, MVT::i16),
            CLI.DAG.getConstant(Offset, CLI.DL, MVT::i16));

        MemOpChains.push_back(CLI.DAG.getStore(
            Chain, CLI.DL, CLI.OutVals[ArgIdx], PtrOff,
            MachinePointerInfo::getStack(MF, VA.getLocMemOffset())));
      } else {
        llvm_unreachable("TODO support for offset greater than 16 bits is not "
                         "implemented yet!");
      }
    } else {
      llvm_unreachable("Unexpected argument location!");
    }
  }

  if (!MemOpChains.empty()) {
    Chain = CLI.DAG.getNode(ISD::TokenFactor, CLI.DL, MVT::Other, MemOpChains);
  }

  SDValue InGlue;
  for (const auto &Reg : RegsToPass) {
    Chain = CLI.DAG.getCopyToReg(Chain, CLI.DL, Reg.first, Reg.second, InGlue);
    InGlue = Chain.getValue(1);
  }

  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);

  // Add the callee to CALL operands. If possible make it legal.
  auto PtrVT = getPointerTy(CLI.DAG.getDataLayout());
  if (auto GANode = dyn_cast<GlobalAddressSDNode>(Callee);
      GANode && isa<Function>(GANode->getGlobal())) {
    Ops.push_back(
        CLI.DAG.getTargetGlobalAddress(GANode->getGlobal(), CLI.DL, PtrVT));
  } else if (auto ESNode = dyn_cast<ExternalSymbolSDNode>(Callee); ESNode) {
    Ops.push_back(CLI.DAG.getTargetExternalSymbol(ESNode->getSymbol(), PtrVT));
  } else {
    Ops.push_back(Callee);
  }

  // Argument registers are added as operands to the call to mark them as
  // live-in.
  for (const auto &Reg : RegsToPass) {
    Ops.push_back(CLI.DAG.getRegister(Reg.first, Reg.second.getValueType()));
  }

  // Add a mask indicating which registers are preserved during the call.
  // This is necessary for register allocator but also for correctly marking
  // retval register as implicit-def - this is done in
  // MachineInstr::setPhysRegsDeadExcept(), which is called from
  // IntrEmitter::EmitMachineNode().
  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask =
      TRI->getCallPreservedMask(CLI.DAG.getMachineFunction(), CLI.CallConv);
  Ops.push_back(CLI.DAG.getRegisterMask(Mask));

  if (InGlue.getNode()) {
    Ops.push_back(InGlue);
  }

  Chain = CLI.DAG.getNode(STM8::CALL, CLI.DL,
                          CLI.DAG.getVTList(MVT::Other, MVT::Glue), Ops);
  InGlue = Chain.getValue(1);

  Chain = CLI.DAG.getCALLSEQ_END(Chain, NumBytes, 0, InGlue, CLI.DL);
  InGlue = Chain.getValue(1);

  return LowerCallResult(Chain, InGlue, CLI.CallConv, CLI.IsVarArg, CLI.Ins,
                         CLI.DL, CLI.DAG, InVals);
}

SDValue STM8TargetLowering::LowerCallResult(
    SDValue Chain, SDValue InGlue, CallingConv::ID CC, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  SmallVector<CCValAssign, 8> RVLocs;
  CCState CCInfo(CC, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, STM8::getRetCCAssignFn(CC));

  for (const CCValAssign &RVLoc : RVLocs) {
    if (RVLoc.isRegLoc()) {
      Chain = DAG.getCopyFromReg(Chain, DL, RVLoc.getLocReg(), RVLoc.getLocVT(),
                                 InGlue)
                  .getValue(1);

      InGlue = Chain.getValue(2);

      InVals.push_back(Chain.getValue(0));
    } else {
      llvm_unreachable("Unexpected return value location!");
    }
  }

  return Chain;
}

SDValue STM8TargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue InChain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Target = Op.getOperand(4);

  SDLoc Loc(Op);

  return getFusedCompareJumpNode(DAG, Loc, InChain, CC, LHS, RHS, Target);
}

SDValue STM8TargetLowering::LowerSELECT_CC(SDValue Op,
                                           SelectionDAG &DAG) const {
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  SDValue CmpLHS = Op.getOperand(0);
  SDValue CmpRHS = Op.getOperand(1);
  SDValue ValIfTrue = Op.getOperand(2);
  SDValue ValIfFalse = Op.getOperand(3);

  SDLoc Loc(Op);

  bool RhsIsZero = isa<ConstantSDNode>(CmpRHS) &&
                   (cast<ConstantSDNode>(CmpRHS)->getZExtValue() == 0);

  unsigned TnzCC = 0;
  unsigned CpCC = getCondCodeConstant(CC, &TnzCC);

  if (RhsIsZero && TnzCC) {
    SDValue CCVal = DAG.getTargetConstant(TnzCC, Loc, MVT::i8);
    return DAG.getNode(STM8::TNZ_SELECT_CC, Loc, ValIfTrue.getValueType(),
                       CmpLHS, ValIfTrue, ValIfFalse, CCVal);
  } else {
    SDValue CCVal = DAG.getTargetConstant(CpCC, Loc, MVT::i8);
    return DAG.getNode(STM8::COMPARE_SELECT_CC, Loc, ValIfTrue.getValueType(),
                       CmpLHS, tryGetTargetConst(DAG, CmpRHS), ValIfTrue,
                       ValIfFalse, CCVal);
  }
}

SDValue STM8TargetLowering::LowerGlobalAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
  GlobalAddressSDNode *AddrNode = cast<GlobalAddressSDNode>(Op);

  const GlobalValue *Address = AddrNode->getGlobal();
  int64_t Offset = AddrNode->getOffset();

  SDLoc DL(Op);
  SDValue TGA = DAG.getTargetGlobalAddress(Address, DL, MVT::i16, Offset);
  return DAG.getNode(STM8::WRAPPER, DL, MVT::i16, TGA);
}

SDValue STM8TargetLowering::LowerJumpTable(SDValue Op,
                                           SelectionDAG &DAG) const {
  JumpTableSDNode *JTNode = cast<JumpTableSDNode>(Op);
  SDValue TJT = DAG.getTargetJumpTable(JTNode->getIndex(), MVT::i16);
  return DAG.getNode(STM8::WRAPPER, SDLoc(Op), MVT::i16, TJT);
}

SDValue STM8TargetLowering::LowerSignExt(SDValue Op, SelectionDAG &DAG) const {
  SDValue InVal = Op.getOperand(0);

  if ((Op.getValueType() != MVT::i16) || (InVal.getValueType() != MVT::i8)) {
    return {};
  }

  SDLoc DL(Op);

  SDValue ZeroWord = DAG.getConstant(0, DL, MVT::i16);
  SDValue AllOnesWord = DAG.getConstant(0xFFFF, DL, MVT::i16);
  SDValue LowByte = DAG.getConstant(0, DL, MVT::i8);

  SDValue CCVal = DAG.getTargetConstant(STM8::COND_MI, DL, MVT::i8);

  SDValue Word = DAG.getNode(STM8::TNZ_SELECT_CC, DL, MVT::i16, InVal,
                             AllOnesWord, ZeroWord, CCVal);

  return DAG.getNode(STM8::SET_BYTE, DL, MVT::i16, Word, LowByte, InVal);
}

static bool isConstByteEqualTo(const SDValue &Val, unsigned ByteIdx,
                               uint8_t ByteVal) {
  if (!isa<ConstantSDNode>(Val)) {
    return false;
  }

  uint64_t CV = cast<ConstantSDNode>(Val)->getZExtValue();
  uint64_t CVByte = (CV >> (ByteIdx * 8u)) & UINT64_C(0xFF);
  return ByteVal == CVByte;
}

static bool isConstByteNull(const SDValue &Val, unsigned ByteIdx) {
  return isConstByteEqualTo(Val, ByteIdx, 0);
}

SDValue STM8TargetLowering::LowerWordLogicalOp(SDValue Op,
                                               SelectionDAG &DAG) const {
  assert(Op.getSimpleValueType() == MVT::i16);

  unsigned Opcode = Op.getOpcode();

  SDLoc DL(Op);
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);

  // For simplicity force constant operand (if any)  to be RHS
  if (isa<ConstantSDNode>(LHS)) {
    std::swap(LHS, RHS);
  }

  SDValue LoIdx = DAG.getConstant(0, DL, MVT::i8);
  SDValue HiIdx = DAG.getConstant(1, DL, MVT::i8);

  SDValue LHSLo = DAG.getNode(STM8::GET_BYTE, DL, MVT::i8, LHS, LoIdx);
  SDValue LHSHi = DAG.getNode(STM8::GET_BYTE, DL, MVT::i8, LHS, HiIdx);
  SDValue RHSLo = DAG.getNode(STM8::GET_BYTE, DL, MVT::i8, RHS, LoIdx);
  SDValue RHSHi = DAG.getNode(STM8::GET_BYTE, DL, MVT::i8, RHS, HiIdx);

  SDValue ResLo = DAG.getNode(Opcode, DL, MVT::i8, LHSLo, RHSLo);
  SDValue ResHi = DAG.getNode(Opcode, DL, MVT::i8, LHSHi, RHSHi);

  // Special case 1: AND, RHS is constant with low byte = 0
  if (Opcode == ISD::AND && isConstByteNull(RHS, 0)) {
    SDValue Zero = DAG.getConstant(0, DL, MVT::i16);
    return DAG.getNode(STM8::SET_BYTE, DL, MVT::i16, Zero, HiIdx, ResHi);
  }

  // Special case 2: AND, RHS is constant with high byte = 0
  if (Opcode == ISD::AND && isConstByteNull(RHS, 1)) {
    SDValue Zero = DAG.getConstant(0, DL, MVT::i16);
    return DAG.getNode(STM8::SET_BYTE, DL, MVT::i16, Zero, LoIdx, ResLo);
  }

  // Special case 3: OR, RHS is constant with low byte = 0
  if (Opcode == ISD::OR && isConstByteNull(RHS, 0)) {
    return DAG.getNode(STM8::SET_BYTE, DL, MVT::i16, LHS, HiIdx, ResHi);
  }

  // Special case 4: OR, RHS is constant with high byte = 0
  if (Opcode == ISD::OR && isConstByteNull(RHS, 1)) {
    return DAG.getNode(STM8::SET_BYTE, DL, MVT::i16, LHS, LoIdx, ResLo);
  }

  SDValue Res = DAG.getNode(STM8::SET_BYTE, DL, MVT::i16,
                            DAG.getUNDEF(MVT::i16), LoIdx, ResLo);
  return DAG.getNode(STM8::SET_BYTE, DL, MVT::i16, Res, HiIdx, ResHi);
}

SDValue STM8TargetLowering::LowerShiftOp(SDValue Op, SelectionDAG &DAG) const {
  unsigned Opcode = Op.getOpcode();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);

  SDLoc DL(Op);

  // Special case #1: (LHS = 0 or RHS = 0) => result = LHS
  if (isNullConstant(LHS) || isNullConstant(RHS)) {
    return LHS;
  }

  // Special case #2: LHS = 1:
  if (isOneConstant(LHS)) {
    // TODO: use a lookup table.
  }

  // Special case #3: RHS is constant:
  if (auto ConstRHS = dyn_cast<ConstantSDNode>(RHS); ConstRHS) {
    uint64_t ShiftBy = ConstRHS->getZExtValue();

    bool IsLogShift = (Opcode == ISD::SHL) || (Opcode == ISD::SRL);
    bool IsWordShift = (Op.getSimpleValueType() == MVT::i16);

    // Special case #3.1: shifts by 8 bits are lowered to SWAPW + AND
    if (IsLogShift && IsWordShift && (ShiftBy == 8)) {
      SDValue SwappedLHS = DAG.getNode(STM8::SWAP_BYTES, DL, MVT::i16, LHS);
      SDValue Zero = DAG.getConstant(0, DL, MVT::i8);

      if (Opcode == ISD::SHL) {
        // Shift left: low byte is moved to high byte, new low byte is set to
        // zero
        return DAG.getNode(STM8::SET_BYTE, DL, MVT::i16, SwappedLHS, Zero,
                           Zero);
      } else {
        // Shift right: high byte is moved to low byte, new high byte is set to
        // zero
        SDValue HiIdx = DAG.getConstant(1, DL, MVT::i8);
        return DAG.getNode(STM8::SET_BYTE, DL, MVT::i16, SwappedLHS, HiIdx,
                           Zero);
      }
    }

    unsigned NewOpcode = 0;
    switch (Opcode) {
    case ISD::SHL:
      NewOpcode = STM8::SHL;
      break;
    case ISD::SRL:
      NewOpcode = STM8::SHR;
      break;
    case ISD::SRA:
      NewOpcode = STM8::ASHR;
      break;
    default:
      llvm_unreachable("Unexpected shift opcode");
    }

    SDValue Res = LHS;
    for (uint64_t i = 0; i < ShiftBy; ++i) {
      Res = DAG.getNode(NewOpcode, DL, LHS.getValueType(), Res);
    }

    return Res;
  }

  // General case: both LHS and RHS are variable.
  unsigned NewOpcode = 0;
  switch (Opcode) {
  case ISD::SHL:
    NewOpcode = STM8::VAR_SHL;
    break;
  case ISD::SRL:
    NewOpcode = STM8::VAR_SHR;
    break;
  case ISD::SRA:
    NewOpcode = STM8::VAR_ASHR;
    break;
  default:
    llvm_unreachable("Unexpected shift opcode");
  }

  return DAG.getNode(NewOpcode, DL, LHS.getValueType(), LHS, RHS);
}

SDValue STM8TargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  auto *FuncInfo = MF.getInfo<STM8MachineFunctionInfo>();

  SDValue InChain = Op.getOperand(0);
  SDValue DstAddr = Op.getOperand(1);
  const Value *SrcVal = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  SDValue VaArgsFI =
      DAG.getFrameIndex(FuncInfo->GetVaArgsFrameIndex(), MVT::i16);

  return DAG.getStore(InChain, SDLoc(Op), VaArgsFI, DstAddr,
                      MachinePointerInfo(SrcVal));
}

SDValue STM8TargetLowering::LowerUDivURem(SDValue Op, SelectionDAG &DAG) const {
  // ISD::UDIV/UREM/UDIVREM always take two operands and return result(s) of the
  // same type. STM8 supports division with reminder of two i16 values or i16
  // dividend and i8 divisor. In order to simplify selection patterns we lower
  // these ISD::* instructions to STM8::UDIVREM with (i16, i16) or (i16, i8)
  // operands.

  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);

  SDLoc DL(Op);
  EVT LVT = LHS.getValueType();

  SDValue UDiv, URem;

  if (LVT == MVT::i8) {
    // Case 1: Division of two i8 values
    SDValue ExtLHS = DAG.getZExtOrTrunc(LHS, DL, MVT::i16);
    SDVTList VTs = DAG.getVTList(MVT::i16, MVT::i8);
    SDValue Res = DAG.getNode(STM8::UDIVREM, DL, VTs, ExtLHS, RHS);

    // Quotient resulting from STM8::UDIVREM is i16 value, we need to truncate
    // it to i8.
    UDiv = DAG.getZExtOrTrunc(Res.getValue(0), DL, MVT::i8);
    URem = Res.getValue(1);
  }

  else if ((LVT == MVT::i16) && (RHS.getOpcode() == ISD::ZERO_EXTEND)) {
    // Case 2: Division of i16 by zero-extended i8
    SDValue TruncRHS = RHS.getOperand(0);
    SDVTList VTs = DAG.getVTList(MVT::i16, MVT::i8);
    SDValue Res = DAG.getNode(STM8::UDIVREM, DL, VTs, LHS, TruncRHS);

    // Reminder from STM8::UDIVREM is i8 so we need to zero-extend it to i16.
    UDiv = Res.getValue(0);
    URem = DAG.getZExtOrTrunc(Res.getValue(1), DL, MVT::i16);
  } else {
    // Case 3: Division of two i16 values
    assert((LVT == MVT::i16) && (RHS.getValueType() == MVT::i16) &&
           "Expected division of two i16 values");
    SDVTList VTs = DAG.getVTList(MVT::i16, MVT::i16);
    SDValue Res = DAG.getNode(STM8::UDIVREM, DL, VTs, LHS, RHS);

    // In this case both quotient and reminder are i16.
    UDiv = Res.getValue(0);
    URem = Res.getValue(1);
  }

  switch (Op.getOpcode()) {
  case ISD::UDIV:
    return UDiv;
  case ISD::UREM:
    return URem;
  case ISD::UDIVREM:
    return DAG.getMergeValues({UDiv, URem}, DL);
  default:
    break;
  }

  llvm_unreachable("Unexpected opcode");
}

MachineBasicBlock *
STM8TargetLowering::emitSelect(MachineInstr &MI, MachineBasicBlock *MBB) const {

  // "Select" instruction has to be transformed into a conditional statement.
  // The result will be equivalent to the following assembly code:
  //
  //   CP $lhs, $rhs   ; Comparison (alternatively TNZ for TnzSelect)
  //   JRxx if_true    ; If the bit in CC corresponding to the condition
  //                   ; code in Select is set, jump to "if_true" block
  // if_false:
  //   LD $dst, $f_val ; Copy the "false value" to the destination register
  //   JRA tail        ; Jump over "if_true" block directly to the "tail" block
  // if_true:
  //   LD $dst, $t_val ; Copy the "true value" to the destination register
  // tail:
  //   ...             ; The rest of the program
  //
  // Here, we create "if_true", "if_false" and "tail" blocks. "if_false"
  // contains only unconditional jump to "tail" (JRA), and "if_true" is empty.
  // PHI Elimination pass will insert appropriate LD instructions to the
  // destination register into these blocks.

  unsigned Opcode = MI.getOpcode();

  LLVM_DEBUG(dbgs() << "Emitting STM8 select machine instructions\n");

  MachineFunction *MF = MBB->getParent();
  const TargetSubtargetInfo &STI = MF->getSubtarget();
  const STM8InstrInfo &TII = (const STM8InstrInfo &)*STI.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  SmallVector<MachineOperand, 2> CmpOps;
  Register TrueVal, FalseVal;
  unsigned CCVal = 0;

  switch (Opcode) {
  case STM8::TnzSelect8r:
  case STM8::TnzSelect16r:
    CmpOps.push_back(MI.getOperand(1));
    TrueVal = MI.getOperand(2).getReg();
    FalseVal = MI.getOperand(3).getReg();
    CCVal = MI.getOperand(4).getImm();
    break;
  case STM8::CompareSelect8ai8:
  case STM8::CompareSelect8ab:
  case STM8::CompareSelect16ri16:
  case STM8::CompareSelect16xz:
    CmpOps.push_back(MI.getOperand(1));
    CmpOps.push_back(MI.getOperand(2));
    TrueVal = MI.getOperand(3).getReg();
    FalseVal = MI.getOperand(4).getReg();
    CCVal = MI.getOperand(5).getImm();
    break;
  default: {
    std::string msg =
        "Unexpected tnz/cp select opcode " + TII.getName(Opcode).str();
    llvm_unreachable(msg.c_str());
  }
  }

  MachineBasicBlock *FalseMBB =
      MF->CreateMachineBasicBlock(MBB->getBasicBlock());
  MachineBasicBlock *TrueMBB =
      MF->CreateMachineBasicBlock(MBB->getBasicBlock());
  MachineBasicBlock *TailMBB = createTailMBB(MBB, MI);

  auto InsertPos = ++MBB->getIterator();
  MF->insert(InsertPos, FalseMBB);
  MF->insert(InsertPos, TrueMBB);

  // Set successor/predecessor relationships. Order matters, FalseMBB must be
  // added first so that control falls through directly to it from MBB. Note:
  // this has to be done after calling transferSuccessorsAndUpdatePHIs()!
  MBB->addSuccessor(FalseMBB);
  MBB->addSuccessor(TrueMBB);
  TrueMBB->addSuccessor(TailMBB);
  FalseMBB->addSuccessor(TailMBB);

  // If condition is met, jump to TrueMBB. Otherwise, fall through to FalseMBB.
  unsigned TnzCpOpcode = 0;
  switch (Opcode) {
  case STM8::TnzSelect8r:
    TnzCpOpcode = STM8::GenTNZr;
    break;
  case STM8::TnzSelect16r:
    TnzCpOpcode = STM8::GenTNZWr;
    break;
  case STM8::CompareSelect8ai8:
    TnzCpOpcode = STM8::CPai8;
    break;
  case STM8::CompareSelect8ab:
    TnzCpOpcode = STM8::PseudoCPab;
    break;
  case STM8::CompareSelect16ri16:
    TnzCpOpcode = STM8::GenCPWri16;
    break;
  case STM8::CompareSelect16xz:
    TnzCpOpcode = STM8::PseudoCPWxz;
    break;
  }

  BuildMI(MBB, DL, TII.get(TnzCpOpcode)).add(CmpOps);
  BuildMI(MBB, DL, TII.getJrCond(CCVal)).addMBB(TrueMBB);

  // FalseMBB has to end with an unconditional jump to over TrueMBB to TailMBB.
  // TrueMBB doesn't need that, as LLVM won't reorder MBBs unless
  // TargetInstrInfo::analyzeBranch() allows it to do so.
  BuildMI(FalseMBB, DL, TII.get(STM8::JRA)).addMBB(TailMBB);

  // Insert PHI at the beginning of TailMBB to choose the result value
  BuildMI(*TailMBB, TailMBB->begin(), DL, TII.get(STM8::PHI),
          MI.getOperand(0).getReg())
      .addReg(TrueVal)
      .addMBB(TrueMBB)
      .addReg(FalseVal)
      .addMBB(FalseMBB);

  // Finally, remove the Select instruction
  MI.removeFromParent();

  return TailMBB;
}

MachineBasicBlock *
STM8TargetLowering::emitVarShift(MachineInstr &MI,
                                 MachineBasicBlock *MBB) const {
  // As STM8 only supports single bit shifts, variable bit shifts have to be
  // implemented as a loop. The following code creates machine basic blocks
  // (MBBs) which correspond to the following assembly code. %r is the register
  // to be shifted, %s is the number of bits:
  //
  //   tnz  %s            ; check if %s is 0
  //   jrne tail          ; skip the loop if %s is 0
  // loop:
  //   sll  %r            ; shift by one bit, sll, srl or sra
  //   dec  %s            ; decrement %s, this sets CC.z bit if %s is 0
  //   jreq loop          ; jump to the next iteration if CC.z is not 0
  // tail:
  //   ; rest of the program
  //
  // There are new 3 MBBs:
  // 1) Loop header, which contains PHI instructions for %r and %s,
  // 2) Loop body,
  // 3) Tail, which contains instructions which were after the shift in the
  //    original MBB.
  //
  // MBB
  //  |
  //  +----> LoopHdrMBB <-+
  //  |       |           |
  //  |      LoopMBB -----+
  //  |       |
  //  +<------+
  //  |
  // Tail

  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetSubtargetInfo &STI = MF->getSubtarget();
  const STM8InstrInfo &TII = (const STM8InstrInfo &)*STI.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  Register Dst = MI.getOperand(0).getReg();
  Register SrcLHS = MI.getOperand(1).getReg();
  Register SrcRHS = MI.getOperand(2).getReg();

  unsigned ShiftOpcode = 0;
  switch (MI.getOpcode()) {
  case STM8::PseudoVarSLL:
    ShiftOpcode = STM8::SLLa;
    break;
  case STM8::PseudoVarSRL:
    ShiftOpcode = STM8::SRLa;
    break;
  case STM8::PseudoVarSRA:
    ShiftOpcode = STM8::SRAa;
    break;
  case STM8::PseudoVarSLLW:
    ShiftOpcode = STM8::GenSLLWr;
    break;
  case STM8::PseudoVarSRLW:
    ShiftOpcode = STM8::GenSRLWr;
    break;
  case STM8::PseudoVarSRAW:
    ShiftOpcode = STM8::GenSRAWr;
    break;
  default:
    llvm_unreachable("Unexpected shift opcode");
  }

  MachineBasicBlock *LoopHdrMBB =
      MF->CreateMachineBasicBlock(MBB->getBasicBlock());
  MachineBasicBlock *LoopMBB =
      MF->CreateMachineBasicBlock(MBB->getBasicBlock());
  MachineBasicBlock *TailMBB = createTailMBB(MBB, MI);

  auto InsertPos = std::next(MBB->getIterator());
  MF->insert(InsertPos, LoopHdrMBB);
  MF->insert(InsertPos, LoopMBB);

  MBB->addSuccessor(LoopHdrMBB);
  MBB->addSuccessor(TailMBB);
  LoopHdrMBB->addSuccessor(LoopMBB);
  LoopMBB->addSuccessor(LoopHdrMBB);
  LoopMBB->addSuccessor(TailMBB);

  // Create registers
  Register LoopLHS = MRI.cloneVirtualRegister(SrcLHS);
  Register LoopRHS = MRI.cloneVirtualRegister(SrcRHS);
  Register LoopRes = MRI.cloneVirtualRegister(Dst);
  Register LoopNewRHS = MRI.cloneVirtualRegister(SrcRHS);

  // MBB: check if the loop must be skipped (%s == 0)
  BuildMI(MBB, DL, TII.get(STM8::GenTNZr)).addReg(SrcRHS);
  BuildMI(MBB, DL, TII.get(STM8::JREQ)).addMBB(TailMBB);

  // Loop header
  BuildMI(LoopHdrMBB, DL, TII.get(STM8::PHI), LoopLHS)
      .addReg(SrcLHS)
      .addMBB(MBB)
      .addReg(LoopRes)
      .addMBB(LoopMBB);
  BuildMI(LoopHdrMBB, DL, TII.get(STM8::PHI), LoopRHS)
      .addReg(SrcRHS)
      .addMBB(MBB)
      .addReg(LoopNewRHS)
      .addMBB(LoopMBB);

  // LoopMBB: shift by one bit, decrement RHS, jump to the header
  BuildMI(LoopMBB, DL, TII.get(ShiftOpcode), LoopRes).addReg(LoopLHS);
  BuildMI(LoopMBB, DL, TII.get(STM8::DECa), LoopNewRHS).addReg(LoopRHS);
  BuildMI(LoopMBB, DL, TII.get(STM8::TNZa)).addReg(LoopNewRHS);
  BuildMI(LoopMBB, DL, TII.get(STM8::JRNE)).addMBB(LoopHdrMBB);

  // TailMBB: insert PHI at the beginning to select between SrcLHS and LoopRes
  BuildMI(*TailMBB, TailMBB->begin(), DL, TII.get(STM8::PHI), Dst)
      .addReg(SrcLHS)
      .addMBB(MBB)
      .addReg(LoopRes)
      .addMBB(LoopMBB);

  // Finally, remove the original shift instruction
  MI.removeFromParent();

  return TailMBB;
}

MachineBasicBlock *
STM8TargetLowering::emitVarShiftParts(MachineInstr &MI,
                                      MachineBasicBlock *MBB) const {
  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetSubtargetInfo &STI = MF->getSubtarget();
  const STM8InstrInfo &TII = (const STM8InstrInfo &)*STI.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  unsigned Opcode = MI.getOpcode();
  Register LoRes = MI.getOperand(0).getReg();
  Register HiRes = MI.getOperand(1).getReg();
  Register LoLHS = MI.getOperand(2).getReg();
  Register HiLHS = MI.getOperand(3).getReg();
  Register ShAmt = MI.getOperand(4).getReg();

  unsigned ShiftOpcode = 0, RotateCarryOpcode = 0;
  bool ShiftRight = false;

  switch (Opcode) {
  case STM8::PseudoVarSLLWParts:
    ShiftOpcode = STM8::GenSLLWr;
    RotateCarryOpcode = STM8::GenRLCWr;
    ShiftRight = false;
    break;
  case STM8::PseudoVarSRLWParts:
    ShiftOpcode = STM8::GenSRLWr;
    RotateCarryOpcode = STM8::GenRRCWr;
    ShiftRight = true;
    break;
  default:
    llvm_unreachable("Unexpected shift opcode");
  }

  MachineBasicBlock *LoopHdrMBB =
      MF->CreateMachineBasicBlock(MBB->getBasicBlock());
  MachineBasicBlock *LoopMBB =
      MF->CreateMachineBasicBlock(MBB->getBasicBlock());
  MachineBasicBlock *TailMBB = createTailMBB(MBB, MI);

  auto InsertPos = std::next(MBB->getIterator());
  MF->insert(InsertPos, LoopHdrMBB);
  MF->insert(InsertPos, LoopMBB);

  MBB->addSuccessor(LoopHdrMBB);
  MBB->addSuccessor(TailMBB);
  LoopHdrMBB->addSuccessor(LoopMBB);
  LoopMBB->addSuccessor(LoopHdrMBB);
  LoopMBB->addSuccessor(TailMBB);

  Register LoopInLoLHS = MRI.cloneVirtualRegister(LoLHS);
  Register LoopInHiLHS = MRI.cloneVirtualRegister(HiLHS);
  Register LoopInShAmt = MRI.cloneVirtualRegister(ShAmt);

  Register LoopOutLoLHS = MRI.cloneVirtualRegister(LoLHS);
  Register LoopOutHiLHS = MRI.cloneVirtualRegister(HiLHS);
  Register LoopOutShAmt = MRI.cloneVirtualRegister(ShAmt);

  // Original MBB:
  // 1) Check if shift amount is 0
  // 2) If shift amount is 0 skip the loop directly to TailMBB
  // 3) Fallthrough to the loop header

  BuildMI(MBB, DL, TII.get(STM8::TNZr_JR))
      .addReg(ShAmt)
      .addImm(STM8::COND_EQ)
      .addMBB(TailMBB);

  // Loop header:
  // 1) PHI nodes for LoLHS, HiLHS and shift amount
  // 4) Fallthrough to LoopMBB

  BuildMI(LoopHdrMBB, DL, TII.get(STM8::PHI), LoopInLoLHS)
      .addReg(LoLHS)
      .addMBB(MBB)
      .addReg(LoopOutLoLHS)
      .addMBB(LoopMBB);
  BuildMI(LoopHdrMBB, DL, TII.get(STM8::PHI), LoopInHiLHS)
      .addReg(HiLHS)
      .addMBB(MBB)
      .addReg(LoopOutHiLHS)
      .addMBB(LoopMBB);
  BuildMI(LoopHdrMBB, DL, TII.get(STM8::PHI), LoopInShAmt)
      .addReg(ShAmt)
      .addMBB(MBB)
      .addReg(LoopOutShAmt)
      .addMBB(LoopMBB);

  // Loop body:
  // 1) Shift the first part (low if shifting left, hight otherwise)
  // 2) Rotate through carry the second part using carry from 1)
  // 3) Decrement shift amount
  // 4) Test new shift amount for equality with 0 using TNZ/TNZW
  // 5) Jump back to the loop header if shift amount is not zero
  // 6) Otherwise fallthrough to TailMBB

  if (ShiftRight) {
    BuildMI(LoopMBB, DL, TII.get(ShiftOpcode), LoopOutHiLHS)
        .addReg(LoopInHiLHS);
    BuildMI(LoopMBB, DL, TII.get(RotateCarryOpcode), LoopOutLoLHS)
        .addReg(LoopInLoLHS);
  } else {
    BuildMI(LoopMBB, DL, TII.get(ShiftOpcode), LoopOutLoLHS)
        .addReg(LoopInLoLHS);
    BuildMI(LoopMBB, DL, TII.get(RotateCarryOpcode), LoopOutHiLHS)
        .addReg(LoopInHiLHS);
  }

  BuildMI(LoopMBB, DL, TII.get(STM8::DECa), LoopOutShAmt).addReg(LoopInShAmt);
  BuildMI(LoopMBB, DL, TII.get(STM8::TNZr_JR))
      .addReg(LoopOutShAmt)
      .addImm(STM8::COND_NE)
      .addMBB(LoopHdrMBB);

  // Tail: add PHIs at the beginning of the block for low and high parts of
  // the result.
  BuildMI(*TailMBB, TailMBB->begin(), DL, TII.get(STM8::PHI), LoRes)
      .addReg(LoLHS)
      .addMBB(MBB)
      .addReg(LoopOutLoLHS)
      .addMBB(LoopHdrMBB);

  BuildMI(*TailMBB, TailMBB->begin(), DL, TII.get(STM8::PHI), HiRes)
      .addReg(HiLHS)
      .addMBB(MBB)
      .addReg(LoopOutHiLHS)
      .addMBB(LoopHdrMBB);

  // Finally remove the original pseudo instruction
  MI.removeFromParent();

  return TailMBB;
}

unsigned STM8TargetLowering::getCondCodeConstant(ISD::CondCode CC,
                                                 unsigned *OutTnzCC) const {

  unsigned CpCC = 0;
  unsigned TnzCC = 0;

  switch (CC) {
  default:
    break;
  case ISD::SETUEQ:
  case ISD::SETEQ:
    CpCC = TnzCC = STM8::COND_EQ;
    break;
  case ISD::SETUGT:
    CpCC = STM8::COND_UGT;
    TnzCC = STM8::COND_NE;
    break;
  case ISD::SETUGE:
    CpCC = STM8::COND_UGE;
    break;
  case ISD::SETULT:
    CpCC = STM8::COND_ULT;
    break;
  case ISD::SETULE:
    CpCC = STM8::COND_ULE;
    break;
  case ISD::SETUNE:
  case ISD::SETNE:
    CpCC = TnzCC = STM8::COND_NE;
    break;
  case ISD::SETGT:
    CpCC = STM8::COND_SGT;
    break;
  case ISD::SETGE:
    CpCC = STM8::COND_SGE;
    TnzCC = STM8::COND_PL;
    break;
  case ISD::SETLT:
    CpCC = STM8::COND_SLT;
    TnzCC = STM8::COND_MI;
    break;
  case ISD::SETLE:
    CpCC = STM8::COND_SLE;
    break;
  }

  if (OutTnzCC) {
    *OutTnzCC = TnzCC;
  }

  return CpCC;
}

SDValue STM8TargetLowering::getFusedCompareJumpNode(
    SelectionDAG &DAG, const SDLoc &DL, const SDValue &InChain,
    ISD::CondCode CC, const SDValue &LHS, const SDValue &RHS,
    const SDValue &JumpTarget) const {

  unsigned TnzCC = 0;
  unsigned CpCC = getCondCodeConstant(CC, &TnzCC);

  const ConstantSDNode *ConstNode = dyn_cast<ConstantSDNode>(RHS);
  bool CmpWithZero = (ConstNode && (0 == ConstNode->getZExtValue()));

  if (CmpWithZero && TnzCC) {
    SDValue CCVal = DAG.getTargetConstant(TnzCC, DL, MVT::i8);
    return DAG.getNode(STM8::TNZ_JUMP, DL, MVT::Other, InChain, LHS, CCVal,
                       JumpTarget);
  } else {
    SDValue CCVal = DAG.getTargetConstant(CpCC, DL, MVT::i8);

    return DAG.getNode(STM8::COMPARE_JUMP, DL, MVT::Other, InChain, LHS,
                       tryGetTargetConst(DAG, RHS), CCVal, JumpTarget);
  }
}

SDValue STM8TargetLowering::tryGetTargetConst(SelectionDAG &DAG,
                                              const SDValue &Val) const {
  const ConstantSDNode *ConstNode = dyn_cast<ConstantSDNode>(Val);
  if (ConstNode) {
    return DAG.getTargetConstant(ConstNode->getSExtValue(), SDLoc(Val),
                                 Val.getValueType());
  }

  return Val;
}

MachineBasicBlock *
STM8TargetLowering::createTailMBB(MachineBasicBlock *MBB,
                                  MachineInstr &splitAfter) const {
  MachineFunction *MF = MBB->getParent();

  MachineBasicBlock *TailMBB =
      MF->CreateMachineBasicBlock(MBB->getBasicBlock());

  MF->insert(std::next(MBB->getIterator()), TailMBB);

  TailMBB->splice(TailMBB->end(), MBB, std::next(splitAfter.getIterator()),
                  MBB->end());
  TailMBB->transferSuccessorsAndUpdatePHIs(MBB);

  return TailMBB;
}

} // namespace llvm
