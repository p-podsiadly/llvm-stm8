#include "STM8Operand.h"
#include "llvm/MC/MCInst.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/MCStreamer.h"

using namespace llvm;

#define GET_REGISTER_MATCHER
#include "STM8GenAsmMatcher.inc"

namespace {

void unLexSavedTokens(MCAsmLexer &Lexer,
                      SmallVectorImpl<AsmToken> &SavedTokens) {
  while (!SavedTokens.empty()) {
    Lexer.UnLex(SavedTokens.pop_back_val());
  }
}

} // namespace

bool STM8Operand::isDirectIndexedX() const {
  return isDirectIndexed() && getReg() == STM8::X;
}

bool STM8Operand::isDirectIndexedY() const {
  return isDirectIndexed() && getReg() == STM8::Y;
}

bool STM8Operand::isDirectOffsetIndexedX() const {
  return isDirectOffsetIndexed() && getReg() == STM8::X;
}
bool STM8Operand::isDirectOffsetIndexedY() const {
  return isDirectOffsetIndexed() && getReg() == STM8::Y;
}
bool STM8Operand::isDirectOffsetIndexedSP() const {
  return isDirectOffsetIndexed() && getReg() == STM8::SP;
}

unsigned STM8Operand::getReg() const {
  switch (Kind) {
  case OperandKind::Register:
    return Oper.Register.RegNo;
  case OperandKind::DirectIndexed:
    return Oper.DirectIndexed.RegNo;
  case OperandKind::DirectOffsetIndexed:
    return Oper.DirectOffsetIndexed.RegNo;
  case OperandKind::IndirectIndexed:
    return Oper.IndirectIndexed.RegNo;
  case OperandKind::Immediate:
  case OperandKind::Token:
  case OperandKind::DirectMem:
  case OperandKind::IndirectMem:
    break;
  }

  return STM8::NoRegister;
}

uint64_t STM8Operand::getImmValue() const {
  switch (Kind) {
  case OperandKind::DirectOffsetIndexed:
    return Oper.DirectOffsetIndexed.Offset;
  case OperandKind::Immediate:
    return Oper.Immediate.Value;
  case OperandKind::DirectMem:
    return Oper.DirectMem.Address;
  case OperandKind::IndirectMem:
    return Oper.IndirectMem.PtrAddr;
  case OperandKind::Register:
  case OperandKind::DirectIndexed:
  case OperandKind::IndirectIndexed:
  case OperandKind::Token:
    break;
  }

  return 0;
}

void STM8Operand::print(raw_ostream &OS) const {
  switch (Kind) {
  case OperandKind::Token:
    OS << "Token: \"" << Token << "\"";
    break;
  case OperandKind::Register:
    OS << "Register: " << getReg();
    break;
  case OperandKind::Immediate:
    OS << "Immediate: \"" << Oper.Immediate.Value << "\"";
    break;
  case OperandKind::DirectMem: {
    OS << "Direct address: \"";
    if (Oper.DirectMem.Expr) {
      OS << *Oper.DirectMem.Expr;
    } else {
      OS << Oper.DirectMem.Address;
    }
    OS << "\"";
    break;
  }
  case OperandKind::DirectIndexed:
    OS << "Direct indexed: register #" << Oper.DirectIndexed.RegNo;
    break;
  case OperandKind::DirectOffsetIndexed:
    OS << "Direct indexed: register #" << Oper.DirectOffsetIndexed.RegNo
       << ", offset = \"" << Oper.DirectOffsetIndexed.Offset << "\"";
    break;
  case OperandKind::IndirectMem:
    OS << "Indirect address: \"" << Oper.IndirectMem.PtrAddr << "\"";
    break;
  case OperandKind::IndirectIndexed:
    OS << "Indirect indexed: register #" << Oper.IndirectIndexed.RegNo
       << ", offset pointer = \"" << Oper.IndirectIndexed.PtrOffset << "\"";
    break;
  }

  OS << "\n";
}

void STM8Operand::addRegOperands(MCInst &Inst, unsigned N) const {
  assert(Kind == OperandKind::Register && "Unexpected operand kind");
  assert(N == 1 && "Invalid number of operands!");

  Inst.addOperand(MCOperand::createReg(Oper.Register.RegNo));
}

void STM8Operand::addImmOperands(MCInst &Inst, unsigned N) const {
  assert(Kind == OperandKind::Immediate && "Unexpected operand kind");
  assert(N == 1 && "Invalid number of operands!");

  Inst.addOperand(MCOperand::createImm(Oper.Immediate.Value));
}

void STM8Operand::addDirectMemOperands(MCInst &Inst, unsigned N) const {
  assert(Kind == OperandKind::DirectMem && "Unexpected operand kind!");
  assert(N == 1 && "Invalid number of operands!");

  if (Oper.DirectMem.Expr) {
    Inst.addOperand(MCOperand::createExpr(Oper.DirectMem.Expr));
  } else {
    Inst.addOperand(MCOperand::createImm(Oper.DirectMem.Address));
  }
}

void STM8Operand::addDirectIndexedOperands(MCInst &Inst, unsigned N) const {
  assert(Kind == OperandKind::DirectIndexed && "Unexpected operand kind!");
  assert(N == 1 && "Invalid number of operands!");

  Inst.addOperand(MCOperand::createReg(getReg()));
}

void STM8Operand::addDirectOffsetIndexedOperands(MCInst &Inst,
                                                 unsigned N) const {
  assert(Kind == OperandKind::DirectOffsetIndexed &&
         "Unexpected operand kind!");
  assert(N == 2 && "Invalid number of operands!");

  unsigned RegNo = getReg();
  uint64_t Offset = getImmValue();

  Inst.addOperand(MCOperand::createReg(RegNo));
  Inst.addOperand(MCOperand::createImm(Offset));
}

void STM8Operand::addIndirectMemOperands(MCInst &Inst, unsigned N) const {
  assert(Kind == OperandKind::IndirectMem && "Unexpected operand kind!");
  assert(N == 1 && "Invalid number of operands!");

  uint64_t Offset = getImmValue();
  Inst.addOperand(MCOperand::createImm(Offset));
}

std::unique_ptr<STM8Operand> STM8Operand::CreateToken(const StringRef &Token,
                                                      const SMLoc &Loc) {
  auto operand =
      std::make_unique<STM8Operand>(OperandKind::Token, SMRange(Loc, Loc));
  operand->Token = Token;
  return operand;
}

std::unique_ptr<STM8Operand> STM8Operand::CreateReg(unsigned RegNo,
                                                    const SMRange &SrcLoc) {
  auto operand = std::make_unique<STM8Operand>(OperandKind::Register, SrcLoc);
  operand->Oper.Register.RegNo = RegNo;
  return operand;
}

std::unique_ptr<STM8Operand> STM8Operand::CreateImm(int64_t Value,
                                                    const SMRange &SrcLoc) {
  auto operand = std::make_unique<STM8Operand>(OperandKind::Immediate, SrcLoc);
  operand->Oper.Immediate.Value = Value;
  return operand;
}

std::unique_ptr<STM8Operand>
STM8Operand::CreateSymbolRef(MCContext &Ctx, MCSymbol *Sym,
                             const SMRange &SrcLoc) {
  auto operand = std::make_unique<STM8Operand>(OperandKind::DirectMem, SrcLoc);
  operand->Oper.DirectMem.Address = 0;
  operand->Oper.DirectMem.Expr = MCSymbolRefExpr::create(Sym, Ctx);
  return operand;
}

std::unique_ptr<STM8Operand>
STM8Operand::CreateDirectMem(int64_t Addr, const SMRange &SrcLoc) {
  auto operand = std::make_unique<STM8Operand>(OperandKind::DirectMem, SrcLoc);
  operand->Oper.DirectMem.Address = Addr;
  operand->Oper.DirectMem.Expr = nullptr;
  return operand;
}

std::unique_ptr<STM8Operand>
STM8Operand::CreateDirectIndexed(unsigned RegNo, const SMRange &SrcLoc) {
  auto operand =
      std::make_unique<STM8Operand>(OperandKind::DirectIndexed, SrcLoc);
  operand->Oper.DirectIndexed.RegNo = RegNo;
  return operand;
}

std::unique_ptr<STM8Operand>
STM8Operand::CreateIndirectMem(int64_t PtrAddr, const SMRange &SrcLoc) {
  auto operand =
      std::make_unique<STM8Operand>(OperandKind::IndirectMem, SrcLoc);
  operand->Oper.IndirectMem.PtrAddr = PtrAddr;
  return operand;
}

std::unique_ptr<STM8Operand>
STM8Operand::CreateDirectOffsetIndexed(int64_t Offset, unsigned RegNo,
                                       const SMRange &SrcLoc) {
  auto operand =
      std::make_unique<STM8Operand>(OperandKind::DirectOffsetIndexed, SrcLoc);
  operand->Oper.DirectOffsetIndexed.Offset = Offset;
  operand->Oper.DirectOffsetIndexed.RegNo = RegNo;
  return operand;
}

std::unique_ptr<STM8Operand>
STM8Operand::CreateIndirectIndexed(int64_t PtrOffset, unsigned RegNo,
                                   const SMRange &SrcLoc) {
  auto operand =
      std::make_unique<STM8Operand>(OperandKind::IndirectIndexed, SrcLoc);
  operand->Oper.IndirectIndexed.PtrOffset = PtrOffset;
  operand->Oper.IndirectIndexed.RegNo = RegNo;
  return operand;
}

std::unique_ptr<STM8Operand> STM8Operand::parseImmediate(MCAsmLexer &Lexer) {
  SmallVector<AsmToken, 2> SavedTokens;

  if (AsmToken::Hash != Lexer.getTok().getKind()) {
    return {};
  }

  SavedTokens.push_back(Lexer.getTok());
  Lexer.Lex();

  int64_t Value = 0;
  SMRange ValueLoc;
  if (!parseInteger(Lexer, Value, nullptr, &ValueLoc)) {
    unLexSavedTokens(Lexer, SavedTokens);
    return {};
  }

  return CreateImm(Value, {SavedTokens.front().getLoc(), ValueLoc.End});
}

std::unique_ptr<STM8Operand> STM8Operand::parseSymbolExpr(MCContext &Ctx,
                                                          MCAsmLexer &Lexer) {
  if (AsmToken::Identifier != Lexer.getTok().getKind()) {
    return {};
  }

  SMRange Loc = Lexer.getTok().getLocRange();
  MCSymbol *Sym = Ctx.getOrCreateSymbol(Lexer.getTok().getString());
  Lexer.Lex();

  return CreateSymbolRef(Ctx, Sym, Loc);
}

std::unique_ptr<STM8Operand> STM8Operand::parseRegister(MCAsmLexer &Lexer) {
  if (AsmToken::Identifier != Lexer.getTok().getKind()) {
    return {};
  }

  unsigned RegNo = MatchRegisterName(Lexer.getTok().getIdentifier().lower());
  if (RegNo == STM8::NoRegister) {
    return {};
  }

  auto Operand = CreateReg(RegNo, Lexer.getTok().getLocRange());
  Lexer.Lex();
  return Operand;
}

std::unique_ptr<STM8Operand>
STM8Operand::parseDirectMemOperand(MCAsmLexer &Lexer) {
  int64_t Addr = 0;
  SMRange SrcLoc;

  if (!parseInteger(Lexer, Addr, nullptr, &SrcLoc)) {
    return {};
  }

  return CreateDirectMem(Addr, SrcLoc);
}

std::unique_ptr<STM8Operand> STM8Operand::parseDirectIndexedOperand(
    const ArrayRef<const unsigned> &AllowedRegs, MCAsmLexer &Lexer,
    bool WithOffset) {
  SmallVector<AsmToken, 5> SavedTokens;

  int64_t Offset = 0;
  unsigned RegNo = 0;

  if (AsmToken::LParen != Lexer.getTok().getKind()) {
    return {};
  }

  SavedTokens.push_back(Lexer.getTok());
  Lexer.Lex();

  if (WithOffset) {
    if (AsmToken::Hash != Lexer.getTok().getKind()) {
      unLexSavedTokens(Lexer, SavedTokens);
      return {};
    }

    SavedTokens.push_back(Lexer.getTok());
    Lexer.Lex();

    if (!parseInteger(Lexer, Offset, &SavedTokens, nullptr)) {
      unLexSavedTokens(Lexer, SavedTokens);
      return {};
    }

    // If we got an offset value then we expect a comma followed by a register
    // name:
    if (AsmToken::Comma != Lexer.getTok().getKind()) {
      unLexSavedTokens(Lexer, SavedTokens);
      return {};
    }

    // Eat the comma
    SavedTokens.push_back(Lexer.getTok());
    Lexer.Lex();
  }

  if (AsmToken::Identifier == Lexer.getTok().getKind()) {
    RegNo = MatchRegisterName(Lexer.getTok().getIdentifier().lower());
    if (RegNo == STM8::NoRegister) {
      unLexSavedTokens(Lexer, SavedTokens);
      return {};
    }

    // If we are expecting specific registers and RegNo is not part of that
    // set then...
    if (!AllowedRegs.empty() &&
        (llvm::find(AllowedRegs, RegNo) == AllowedRegs.end())) {
      // ...unlex and report no match.
      unLexSavedTokens(Lexer, SavedTokens);
      return {};
    }

    SavedTokens.push_back(Lexer.getTok());
    Lexer.Lex();
  } else {
    unLexSavedTokens(Lexer, SavedTokens);
    return {};
  }

  if (AsmToken::RParen != Lexer.getTok().getKind()) {
    unLexSavedTokens(Lexer, SavedTokens);
    return {};
  }

  auto SrcLoc =
      SMRange(SavedTokens.front().getLoc(), Lexer.getTok().getEndLoc());
  Lexer.Lex();

  if (WithOffset) {
    return CreateDirectOffsetIndexed(Offset, RegNo, SrcLoc);
  } else {
    return CreateDirectIndexed(RegNo, SrcLoc);
  }
}

std::unique_ptr<STM8Operand>
STM8Operand::parseIndirectMemOperand(MCAsmLexer &Lexer) {
  SmallVector<AsmToken, 3> SavedTokens;

  if (AsmToken::LBrac != Lexer.getTok().getKind()) {
    return {};
  }

  SavedTokens.push_back(Lexer.getTok());
  Lexer.Lex();

  int64_t Addr = 0;
  if (!parseInteger(Lexer, Addr, &SavedTokens, nullptr)) {
    unLexSavedTokens(Lexer, SavedTokens);
    return {};
  }

  if (AsmToken::RBrac != Lexer.getTok().getKind()) {
    unLexSavedTokens(Lexer, SavedTokens);
    return {};
  }

  auto Operand = CreateIndirectMem(
      Addr, {SavedTokens.front().getLoc(), Lexer.getTok().getEndLoc()});

  Lexer.Lex();

  return Operand;
}

std::unique_ptr<STM8Operand>
STM8Operand::parseIndirectIndexedOperand(MCAsmLexer &Lexer) {
  // TODO
  return {};
}

bool STM8Operand::parseInteger(MCAsmLexer &Lexer, int64_t &Value,
                               SmallVectorImpl<AsmToken> *SavedTokens,
                               SMRange *SrcLoc) {
  if (AsmToken::Integer == Lexer.getTok().getKind()) {
    Value = Lexer.getTok().getIntVal();

    if (SavedTokens) {
      SavedTokens->push_back(Lexer.getTok());
    }

    if (SrcLoc) {
      *SrcLoc = Lexer.getTok().getLocRange();
    }

    Lexer.Lex();

    return true;
  }

  return false;
}
