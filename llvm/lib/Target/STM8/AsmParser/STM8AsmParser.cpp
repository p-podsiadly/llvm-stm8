#include "STM8AsmParser.h"

#include "STM8RegisterInfo.h"
#include "TargetInfo/STM8TargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

#define DEBUG_TYPE "stm8-asm-parser"

using namespace llvm;

#define GET_REGISTER_MATCHER
#include "STM8GenAsmMatcher.inc"

// Forward declaration of auto-generated functions.
static void applyMnemonicAliases(StringRef &Mnemonic,
                                 const FeatureBitset &Features,
                                 unsigned VariantID);

STM8AsmParser::STM8AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                             const MCInstrInfo &MII,
                             const MCTargetOptions &Options)
    : MCTargetAsmParser(Options, STI, MII), Parser(Parser) {
  MCAsmParserExtension::Initialize(Parser);
}

bool STM8AsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                     SMLoc NameLoc, OperandVector &Operands) {
  applyMnemonicAliases(Name, getAvailableFeatures(), /*VariantID=*/0);

  Operands.push_back(
      STM8Operand::CreateToken(Name, getLexer().getTok().getLoc()));

  bool First = true;
  while (Parser.getTok().isNot(AsmToken::EndOfStatement)) {
    if (!First) {
      eatComma();
    } else {
      First = false;
    }

    ParseStatus MatchResult = MatchOperandParserImpl(Operands, Name);
    if (MatchResult.isSuccess()) {
      continue;
    }

    auto Operand = STM8Operand::parseImmediate(getLexer());
    if (Operand) {
      Operands.push_back(std::move(Operand));
      continue;
    }

    Operand = STM8Operand::parseRegister(getLexer());
    if (Operand) {
      Operands.push_back(std::move(Operand));
      continue;
    }

    Operand = STM8Operand::parseSymbolExpr(getContext(), getLexer());
    if (Operand) {
      Operands.push_back(std::move(Operand));
      continue;
    }

    SMLoc Loc = getLexer().getLoc();
    Parser.eatToEndOfStatement();
    return Error(Loc, "unexpected token parsing operands");
  }

  // Eat EndOfStatement.
  Parser.Lex();
  return false;
}

bool STM8AsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                  SMLoc &EndLoc) {
  unsigned RegNo = STM8::NoRegister;
  SMRange RegLoc;

  if (tryParseRegName(RegNo, RegLoc, /*RestoreOnFailure=*/false)) {
    Reg = MCRegister(RegNo);
    StartLoc = RegLoc.Start;
    EndLoc = RegLoc.End;
    return false;
  }

  return true;
}

ParseStatus STM8AsmParser::tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                            SMLoc &EndLoc) {
  unsigned RegNo = STM8::NoRegister;
  SMRange RegLoc;

  if (tryParseRegName(RegNo, RegLoc, /*RestoreOnFailure=*/true)) {
    Reg = MCRegister(RegNo);
    StartLoc = RegLoc.Start;
    EndLoc = RegLoc.End;
    return ParseStatus::Success;
  }

  return ParseStatus::NoMatch;
}

bool STM8AsmParser::MatchAndEmitInstruction(SMLoc Loc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);

  switch (MatchResult) {
  case Match_Success:
    Inst.setLoc(Loc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_InvalidOperand:
    return Error(Loc, "Invalid operand #" + std::to_string(ErrorInfo));
  case Match_InvalidTiedOperand:
    return Error(Loc, "Invalid tied operand");
  case Match_MissingFeature:
    return Error(Loc, "Missing feature");
  case Match_MnemonicFail:
    return Error(Loc, "Failed to match mnemonic");
  default:
    break;
  }

  return Error(Loc, "Failed to match the instruction");
}

bool STM8AsmParser::areEqualRegs(const MCParsedAsmOperand &Op1,
                                 const MCParsedAsmOperand &Op2) const {
  unsigned Reg1 = Op1.getReg();
  unsigned Reg2 = Op2.getReg();

  return (Reg1 != 0) && (Reg1 == Reg2);
}

void STM8AsmParser::eatComma() {
  if (getLexer().getTok().is(AsmToken::Comma)) {
    Parser.Lex();
  }
}

bool STM8AsmParser::tryParseRegName(unsigned &RegNo, SMRange &RegLoc,
                                    bool RestoreOnFailure) {
  const AsmToken &Tok = getLexer().getTok();

  if (AsmToken::Identifier != Tok.getKind()) {
    if (!RestoreOnFailure) {
      Lex();
    }

    return STM8::NoRegister;
  }

  unsigned RegNum = MatchRegisterName(Tok.getIdentifier().lower());

  Lex();

  return RegNum;
}

bool STM8AsmParser::tryParseImmediateVal(unsigned &Value, SMRange &SrcLoc) {
  if (AsmToken::Hash != getLexer().getTok().getKind()) {
    return false;
  }

  const SMLoc StartLoc = getLexer().getTok().getLoc();
  AsmToken HashTok = getLexer().getTok();
  Lex();

  if (tryParseDirectAddr(Value, SrcLoc)) {
    SrcLoc.Start = StartLoc;
    return true;
  }

  getLexer().UnLex(HashTok);
  return false;
}

bool STM8AsmParser::tryParseDirectAddr(unsigned &Addr, SMRange &SrcLoc) {
  if (AsmToken::Dollar != getLexer().getTok().getKind()) {
    return false;
  }

  AsmToken ValueTok = getLexer().peekTok();
  if (AsmToken::Integer != ValueTok.getKind()) {
    return false;
  }

  Addr = ValueTok.getIntVal();
  SrcLoc = SMRange(getLexer().getTok().getLoc(), ValueTok.getEndLoc());

  Lex();

  return true;
}

bool STM8AsmParser::tryParseRegIndexed(unsigned &RegNo, SMRange &SrcLoc) {
  if (AsmToken::LParen != getLexer().getTok().getKind()) {
    return false;
  }

  AsmToken LParenTok = getLexer().getTok();

  if (AsmToken::Identifier != Lex().getKind()) {
    getLexer().UnLex(LParenTok);
    return false;
  }

  RegNo = MatchRegisterName(getLexer().getTok().getIdentifier().lower());
  if (RegNo == STM8::NoRegister) {
    getLexer().UnLex(LParenTok);
    return false;
  }

  if (AsmToken::RParen != getLexer().peekTok().getKind()) {
    getLexer().UnLex(LParenTok);
    return false;
  }

  // Move to the ")" token and set source range
  SrcLoc = SMRange(LParenTok.getLoc(), Lex().getEndLoc());

  // Move to the next token after ")"
  Lex();

  return true;
}

bool STM8AsmParser::tryParseOffsetRegIndexed(unsigned &Offset,
                                             SMRange &OffsetSrcLoc,
                                             unsigned &RegNo,
                                             SMRange &RegSrcLoc) {
  if (AsmToken::LParen != getLexer().getTok().getKind()) {
    return false;
  }

  SmallVector<AsmToken, 5> SavedTokens;

  // Save the token in case we need to un-lex and move to the next token
  SavedTokens.push_back(getLexer().getTok());
  Lex();

  if (AsmToken::Dollar != getLexer().getTok().getKind()) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  SavedTokens.push_back(getLexer().getTok());
  Lex();

  if (AsmToken::Integer != getLexer().getTok().getKind()) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  Offset = getLexer().getTok().getIntVal();
  OffsetSrcLoc = getLexer().getTok().getLocRange();

  SavedTokens.push_back(getLexer().getTok());
  Lex();

  if (AsmToken::Comma != getLexer().getTok().getKind()) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  SavedTokens.push_back(getLexer().getTok());
  Lex();

  if (AsmToken::Identifier != getLexer().getTok().getKind()) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  RegNo = MatchRegisterName(getLexer().getTok().getIdentifier().lower());
  if (RegNo == STM8::NoRegister) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  RegSrcLoc = getLexer().getTok().getLocRange();

  if (AsmToken::RParen != getLexer().peekTok().getKind()) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  Lex();

  return true;
}

bool STM8AsmParser::tryParseIndirectAddr(unsigned &PtrAddr,
                                         SMRange &PtrAddrLoc) {
  if (AsmToken::LBrac != getLexer().getTok().getKind()) {
    return false;
  }

  SmallVector<AsmToken, 2> SavedTokens;

  SavedTokens.push_back(getLexer().getTok());
  Lex();

  if (AsmToken::Dollar != getLexer().getTok().getKind()) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  SavedTokens.push_back(getLexer().getTok());
  Lex();

  if (AsmToken::Integer != getLexer().getTok().getKind()) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  PtrAddr = getLexer().getTok().getIntVal();
  PtrAddrLoc = getLexer().getTok().getLocRange();

  if (AsmToken::RBrac != getLexer().peekTok().getKind()) {
    UnLexSavedTokens(SavedTokens);
    return false;
  }

  Lex();

  return true;
}

bool STM8AsmParser::tryParseIndirectOffsetRegIndexed(unsigned &PtrAddr,
                                                     SMRange &PtrAddrSrcLoc,
                                                     unsigned &RegNo,
                                                     SMRange &RegSrcLoc) {
  // TODO
  return false;
}

ParseStatus STM8AsmParser::parseDirectMemOperand(OperandVector &Operands) {
  auto Operand = STM8Operand::parseDirectMemOperand(getLexer());
  if (!Operand) {
    return ParseStatus::NoMatch;
  }

  Operands.push_back(std::move(Operand));
  return ParseStatus::Success;
}

ParseStatus STM8AsmParser::parseDirectIndexedOperand(
    const ArrayRef<const unsigned> &AllowedRegs, OperandVector &Operands) {
  auto Operand = STM8Operand::parseDirectIndexedOperand(AllowedRegs, getLexer(),
                                                        /*WithOffset=*/false);
  if (!Operand) {
    return ParseStatus::NoMatch;
  }

  Operands.push_back(std::move(Operand));
  return ParseStatus::Success;
}

ParseStatus STM8AsmParser::parseDirectIndexedOperandX(OperandVector &Operands) {
  return parseDirectIndexedOperand(STM8::X, Operands);
}

ParseStatus STM8AsmParser::parseDirectIndexedOperandY(OperandVector &Operands) {
  return parseDirectIndexedOperand(STM8::Y, Operands);
}

ParseStatus
STM8AsmParser::parseDirectIndexedOperandXY(OperandVector &Operands) {
  const unsigned Regs[] = {STM8::X, STM8::Y};
  return parseDirectIndexedOperand(Regs, Operands);
}

ParseStatus STM8AsmParser::parseDirectOffsetIndexedOperand(
    const ArrayRef<const unsigned> &AllowedRegs, OperandVector &Operands) {
  auto Operand = STM8Operand::parseDirectIndexedOperand(AllowedRegs, getLexer(),
                                                        /*WithOffset=*/true);
  if (!Operand) {
    return ParseStatus::NoMatch;
  }

  Operands.push_back(std::move(Operand));
  return ParseStatus::Success;
}

ParseStatus
STM8AsmParser::parseDirectOffsetIndexedOperandX(OperandVector &Operands) {
  return parseDirectOffsetIndexedOperand(STM8::X, Operands);
}

ParseStatus
STM8AsmParser::parseDirectOffsetIndexedOperandY(OperandVector &Operands) {
  return parseDirectOffsetIndexedOperand(STM8::Y, Operands);
}

ParseStatus
STM8AsmParser::parseDirectOffsetIndexedOperandSP(OperandVector &Operands) {
  return parseDirectOffsetIndexedOperand(STM8::SP, Operands);
}

ParseStatus
STM8AsmParser::parseDirectOffsetIndexedOperandXY(OperandVector &Operands) {
  const unsigned Regs[] = {STM8::X, STM8::Y};
  return parseDirectOffsetIndexedOperand(Regs, Operands);
}

ParseStatus STM8AsmParser::parseIndirectMemOperand(OperandVector &Operands) {
  auto Operand = STM8Operand::parseIndirectMemOperand(getLexer());
  if (!Operand) {
    return ParseStatus::NoMatch;
  }

  Operands.push_back(std::move(Operand));
  return ParseStatus::Success;
}

ParseStatus
STM8AsmParser::parseIndirectIndexedOperand(OperandVector &Operands) {
  auto Operand = STM8Operand::parseIndirectIndexedOperand(getLexer());
  if (!Operand) {
    return ParseStatus::NoMatch;
  }

  Operands.push_back(std::move(Operand));
  return ParseStatus::Success;
}

void STM8AsmParser::UnLexSavedTokens(SmallVectorImpl<AsmToken> &Tokens) {
  while (!Tokens.empty()) {
    getLexer().UnLex(Tokens.pop_back_val());
  }
}

#define GET_MATCHER_IMPLEMENTATION
#include "STM8GenAsmMatcher.inc"

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSTM8AsmParser() {
  RegisterMCAsmParser<STM8AsmParser> X(getTheSTM8Target());
}
