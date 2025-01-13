#ifndef LLVM_TARGET_STM8_ASM_PARSER_H_INCLUDED
#define LLVM_TARGET_STM8_ASM_PARSER_H_INCLUDED

#include "STM8Operand.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"

namespace llvm {

/**
 * @brief Target-specific implementation of assembly parsing for STM8.
 *
 * Instances of this class are used for target-specific details of MCAsmParser
 * class.
 */
class STM8AsmParser : public MCTargetAsmParser {
public:
  STM8AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                const MCInstrInfo &MII, const MCTargetOptions &Options);

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;

  /**
   * @brief Try to parse a register identifier.
   * @return False on success.
   */
  ParseStatus tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                               SMLoc &EndLoc) override;

  bool MatchAndEmitInstruction(SMLoc Loc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  bool areEqualRegs(const MCParsedAsmOperand &Op1,
                    const MCParsedAsmOperand &Op2) const override;

private:
  enum class OperandParseRes {
    Match,
    NoMatch,
    Error,
  };

  MCAsmParser &Parser;

  MCAsmLexer &getLexer() { return Parser.getLexer(); }

  void eatComma();

  bool tryParseRegName(unsigned &RegNo, SMRange &SrcLoc,
                       bool RestoreOnFailure = true);
  bool tryParseImmediateVal(unsigned &Value, SMRange &SrcLoc);
  bool tryParseDirectAddr(unsigned &Addr, SMRange &SrcLoc);
  bool tryParseRegIndexed(unsigned &RegNo, SMRange &SrcLoc);
  bool tryParseOffsetRegIndexed(unsigned &Offset, SMRange &OffsetSrcLoc,
                                unsigned &RegNo, SMRange &RegSrcLoc);
  bool tryParseIndirectAddr(unsigned &PtrAddr, SMRange &PtrAddrLoc);
  bool tryParseIndirectOffsetRegIndexed(unsigned &PtrAddr,
                                        SMRange &PtrAddrSrcLoc, unsigned &RegNo,
                                        SMRange &RegSrcLoc);

  ParseStatus parseDirectMemOperand(OperandVector &Operands);

  ParseStatus
  parseDirectIndexedOperand(const ArrayRef<const unsigned> &AllowedRegs,
                            OperandVector &Operands);
  ParseStatus parseDirectIndexedOperandX(OperandVector &Operands);
  ParseStatus parseDirectIndexedOperandY(OperandVector &Operands);
  ParseStatus parseDirectIndexedOperandXY(OperandVector &Operands);

  ParseStatus
  parseDirectOffsetIndexedOperand(const ArrayRef<const unsigned> &AllowedRegs,
                                  OperandVector &Operands);
  ParseStatus parseDirectOffsetIndexedOperandX(OperandVector &Operands);
  ParseStatus parseDirectOffsetIndexedOperandY(OperandVector &Operands);
  ParseStatus parseDirectOffsetIndexedOperandSP(OperandVector &Operands);
  ParseStatus parseDirectOffsetIndexedOperandXY(OperandVector &Operands);

  ParseStatus parseIndirectMemOperand(OperandVector &Operands);
  ParseStatus parseIndirectIndexedOperand(OperandVector &Operands);

  void UnLexSavedTokens(SmallVectorImpl<AsmToken> &Tokens);

#define GET_ASSEMBLER_HEADER
#include "STM8GenAsmMatcher.inc"
};

} // namespace llvm

#endif // LLVM_TARGET_STM8_ASM_PARSER_H_INCLUDED
