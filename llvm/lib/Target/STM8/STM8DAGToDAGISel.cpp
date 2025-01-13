#include "STM8DAGToDAGISel.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "llvm/MC/MCInstrDesc.h"

using namespace llvm;

#define GET_DAGISEL_BODY STM8DAGToDAGISel
#include "STM8GenDAGISel.inc"

#define DEBUG_TYPE "stm8-isel"
#define PASS_NAME "STM8 DAG->DAG Instruction Selection"

char STM8DAGToDAGISel::ID = {};

INITIALIZE_PASS(STM8DAGToDAGISel, DEBUG_TYPE, PASS_NAME, false, false)

STM8DAGToDAGISel::STM8DAGToDAGISel(STM8TargetMachine &TM,
                                   CodeGenOptLevel OptLevel)
    : SelectionDAGISel(ID, TM, OptLevel) {
  // TODO
}

void STM8DAGToDAGISel::Select(SDNode *node) {
  unsigned Opcode = node->getOpcode();
  switch (Opcode) {
  case ISD::FrameIndex:
    selectLoadEffectiveAddr(node);
    return;
  case ISD::UMUL_LOHI:
  case ISD::MULHU:
    if (selectMul(node)) {
      return;
    }
    break;
  default:
    break;
  }

  SelectCode(node);
}

STM8AddrMode::AMType STM8DAGToDAGISel::GetAddrMode(SDValue N,
                                                   STM8AddrMode &AM) const {
  AM = {};

  if (isa<ConstantSDNode>(N) || isa<GlobalAddressSDNode>(N) ||
      isa<ExternalSymbolSDNode>(N)) {
    AM.Type = STM8AddrMode::DirectMem;
    AM.Base = N;
    AM.Offset = {};
    return AM.Type;
  }

  if (auto LN = dyn_cast<LoadSDNode>(N); LN) {
    STM8AddrMode IndirectAM;
    GetAddrMode(LN->getOperand(0), IndirectAM);
    if (IndirectAM.Type == STM8AddrMode::DirectMem) {
      AM.Type = STM8AddrMode::IndirectMem;
      AM.Base = AM.Base;
      return AM.Type;
    } else if (IndirectAM.Type != STM8AddrMode::None) {
      AM.Type = STM8AddrMode::Indexed;
      AM.Base = N;
      return AM.Type;
    }
  }

  if (auto FIN = dyn_cast<FrameIndexSDNode>(N); FIN) {
    AM.Type = STM8AddrMode::FrameIndex;
    AM.Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i16);
    AM.Offset = CurDAG->getTargetConstant(0, SDLoc(N), MVT::i8);
    return AM.Type;
  }

  switch (N->getOpcode()) {
  case ISD::ADD: {
    auto LHS = N.getOperand(0);
    auto RHS = N.getOperand(1);

    if (isa<ConstantSDNode>(RHS)) {
      uint64_t RHSVal = cast<ConstantSDNode>(RHS)->getZExtValue();
      SDValue Offset =
          CurDAG->getTargetConstant(RHSVal, SDLoc(RHS), RHS.getValueType());

      if (auto FIN = dyn_cast<FrameIndexSDNode>(LHS); FIN) {
        AM.Type = STM8AddrMode::FrameIndex;
        AM.Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i16);
        AM.Offset = Offset;
      } else if (auto RN = dyn_cast<RegisterSDNode>(LHS);
                 RN && (RN->getReg() == STM8::SP)) {
        AM.Type = STM8AddrMode::SPIndexed;
        AM.Base = LHS;
        AM.Offset = Offset;
      } else {
        AM.Type = STM8AddrMode::Indexed;
        AM.Base = LHS;
        AM.Offset = RHSVal ? Offset : SDValue{};
      }

      return AM.Type;
    } else {
      AM.Type = STM8AddrMode::Indexed;
      AM.Base = N;
      return AM.Type;
    }
  }
  default: {
    AM.Type = STM8AddrMode::Indexed;
    AM.Base = N;
    return AM.Type;
  }
  }

  return STM8AddrMode::None;
}

void STM8DAGToDAGISel::selectLoadEffectiveAddr(SDNode *Node) {
  auto FIN = dyn_cast<FrameIndexSDNode>(Node);
  auto TFIN = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i16);
  auto Disp = CurDAG->getTargetConstant(0, SDLoc(Node), MVT::i8);

  CurDAG->SelectNodeTo(Node, STM8::LoadEffectiveAddr, MVT::i16, TFIN, Disp);
}

bool STM8DAGToDAGISel::selectMul(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();
  assert((Opcode == ISD::UMUL_LOHI) || (Opcode == ISD::MULHU));

  if (Node->getValueType(0) != MVT::i8) {
    return false;
  }

  SDLoc DL(Node);

  SDValue ExpLHS = CurDAG->getAnyExtOrTrunc(Node->getOperand(0), DL, MVT::i16);
  SDValue MulNode = SDValue(CurDAG->getMachineNode(STM8::GenMULra, DL, MVT::i16,
                                                   ExpLHS, Node->getOperand(1)),
                            0);

  SDValue ResLo =
      SDValue(CurDAG->getMachineNode(STM8::GenLDarl, DL, MVT::i8, MulNode), 0);
  SDValue ResHi =
      SDValue(CurDAG->getMachineNode(STM8::GenLDarh, DL, MVT::i8, MulNode), 0);

  if (Node->hasAnyUseOfValue(0)) {
    ReplaceUses(SDValue(Node, 0), ResLo);
  }

  if (Node->hasAnyUseOfValue(1)) {
    ReplaceUses(SDValue(Node, 1), ResHi);
  }

  CurDAG->RemoveDeadNode(Node);

  return true;
}

bool STM8DAGToDAGISel::SelectIR16(SDValue N, SDValue &Base) const {
  STM8AddrMode AM;
  if (STM8AddrMode::Indexed != GetAddrMode(N, AM)) {
    return false;
  }

  if (!AM.Offset) {
    Base = AM.Base;
    return true;
  }

  return false;
}

bool STM8DAGToDAGISel::SelectXYDisp(SDNode *Parent, SDValue Addr, SDValue &Base,
                                    SDValue &Disp) const {
  STM8AddrMode AM;
  if (STM8AddrMode::Indexed != GetAddrMode(Addr, AM)) {
    return false;
  }

  if (AM.Offset) {
    Base = AM.Base;
    Disp = AM.Offset;
    return true;
  }

  return false;
}

bool STM8DAGToDAGISel::SelectSPDisp(SDNode *Parent, SDValue Addr, SDValue &Base,
                                    SDValue &Disp) const {
  STM8AddrMode AM;
  GetAddrMode(Addr, AM);

  if ((AM.Type == STM8AddrMode::SPIndexed) ||
      (AM.Type == STM8AddrMode::FrameIndex)) {
    Base = AM.Base;
    Disp = AM.Offset;
    return true;
  }

  return false;
}
