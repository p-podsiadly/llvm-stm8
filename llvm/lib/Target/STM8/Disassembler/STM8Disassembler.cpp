#include "STM8Disassembler.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "TargetInfo/STM8TargetInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;
using DecodeStatus = MCDisassembler::DecodeStatus;

#define DEBUG_TYPE "stm8-disassembler"

namespace {

template <unsigned N>
bool readInstruction(ArrayRef<uint8_t> Bytes, uint64_t &Size, uint64_t &Instr) {
  if (Bytes.size() < N) {
    return false;
  }

  Size = N;

  Instr = 0;
  for (unsigned BIdx = 0; BIdx < N; ++BIdx) {
    uint64_t B = Bytes[BIdx];
    Instr <<= 8;
    Instr |= static_cast<uint64_t>(B);
  }

  return true;
}

enum OpDecOpcodes {
  OpDec_A,
  OpDec_X,
  OpDec_XL,
  OpDec_XH,
  OpDec_Y,
  OpDec_YL,
  OpDec_YH,
  OpDec_SP,
  OpDec_CC,
  OpDec_Imm3,
  OpDec_Imm8,
  OpDec_Imm16,
  OpDec_Imm24,
  OpDec_Fail,
};

DecodeStatus decodeOperandsImpl(MCInst &, uint64_t &) {
  return DecodeStatus::Success;
}

template <typename H, typename... T>
DecodeStatus decodeOperandsImpl(MCInst &Inst, uint64_t Bits, H OPC, T... Rest) {

#define CASE_REG(reg)                                                          \
  case OpDec_##reg:                                                            \
    Inst.addOperand(MCOperand::createReg(STM8::reg));                          \
    break;
#define CASE_IMM(immBits)                                                      \
  case OpDec_Imm##immBits:                                                     \
    Inst.addOperand(MCOperand::createImm(Bits & ((1u << immBits) - 1)));       \
    Bits >>= immBits;                                                          \
    break;

  switch (OPC) {
    CASE_REG(A)
    CASE_REG(X)
    CASE_REG(Y)
    CASE_REG(XL)
    CASE_REG(XH)
    CASE_REG(YL)
    CASE_REG(YH)
    CASE_REG(SP)
    CASE_REG(CC)
    CASE_IMM(3)
    CASE_IMM(8)
    CASE_IMM(16)
    CASE_IMM(24)
  case OpDec_Fail:
    return DecodeStatus::SoftFail;
  }

#undef CASE_REG
#undef CASE_IMM

  return decodeOperandsImpl(Inst, Bits, Rest...);
}

template <OpDecOpcodes... OPCs>
DecodeStatus decodeOperands(MCInst &Inst, uint64_t Bits, uint64_t,
                            const MCDisassembler *) {
  return decodeOperandsImpl(Inst, Bits, OPCs...);
}

} // namespace

#include "STM8GenDisassemblerTable.inc"

namespace {

const uint8_t *getDecodeTable(uint64_t InstrSize) {
  switch (InstrSize) {
  case 1:
    return DecoderTableSTM88;
  case 2:
    return DecoderTableSTM816;
  case 3:
    return DecoderTableSTM824;
  case 4:
    return DecoderTableSTM832;
  case 5:
    return DecoderTableSTM840;
  default:
    break;
  }

  llvm_unreachable("Unexpected instruction size");
}

} // namespace

STM8Disassembler::STM8Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
    : MCDisassembler(STI, Ctx) {}

MCDisassembler::DecodeStatus
STM8Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                 ArrayRef<uint8_t> Bytes, uint64_t Address,
                                 raw_ostream &CStream) const {
  const auto &STI = getSubtargetInfo();

  uint64_t EncodedInstr = 0;
  if (readInstruction<1>(Bytes, Size, EncodedInstr)) {
    if (Success == decodeInstruction(getDecodeTable(1), Instr, EncodedInstr,
                                     Address, this, STI)) {
      return Success;
    }
  }

  if (readInstruction<2>(Bytes, Size, EncodedInstr)) {
    if (Success == decodeInstruction(getDecodeTable(2), Instr, EncodedInstr,
                                     Address, this, STI)) {
      return Success;
    }
  }

  if (readInstruction<3>(Bytes, Size, EncodedInstr)) {
    if (Success == decodeInstruction(getDecodeTable(3), Instr, EncodedInstr,
                                     Address, this, STI)) {
      return Success;
    }
  }

  if (readInstruction<4>(Bytes, Size, EncodedInstr)) {
    if (Success == decodeInstruction(getDecodeTable(4), Instr, EncodedInstr,
                                     Address, this, STI)) {
      return Success;
    }
  }

  if (readInstruction<5>(Bytes, Size, EncodedInstr)) {
    if (Success == decodeInstruction(getDecodeTable(5), Instr, EncodedInstr,
                                     Address, this, STI)) {
      return Success;
    }
  }

  return Fail;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSTM8Disassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(
      getTheSTM8Target(),
      [](const Target &T, const MCSubtargetInfo &STI, MCContext &Ctx)
          -> MCDisassembler * { return new STM8Disassembler(STI, Ctx); });
}
