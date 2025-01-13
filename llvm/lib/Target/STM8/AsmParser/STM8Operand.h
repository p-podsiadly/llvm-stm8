#ifndef LLVM_TARGET_STM8_OPERAND_H_INCLUDED
#define LLVM_TARGET_STM8_OPERAND_H_INCLUDED

#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"

namespace llvm {

/**
 * @brief Operand parsed from STM8 assembly code.
 *
 * This class is used from code generated from TableGen files (see
 * STM8GenAsmMatcher.inc).
 *
 * Instances of this class are created using STM8Operand::CreateToken() static
 * method
 */
class STM8Operand : public MCParsedAsmOperand {
public:
  enum class OperandKind {
    Immediate,
    Register,
    Token,
    DirectMem,
    DirectIndexed,
    DirectOffsetIndexed,
    IndirectMem,
    IndirectIndexed,
  };

  explicit STM8Operand(OperandKind Kind, const SMRange &SrcLoc)
      : Kind(Kind), SrcLoc(SrcLoc) {}

  bool isReg() const override { return Kind == OperandKind::Register; }
  bool isImm() const override { return Kind == OperandKind::Immediate; }
  bool isToken() const override { return Kind == OperandKind::Token; }
  bool isMem() const override { /* TODO */
    return isDirectMem() || isIndirectMem();
  }

  bool isDirectMem() const { return Kind == OperandKind::DirectMem; }

  bool isDirectIndexed() const { return Kind == OperandKind::DirectIndexed; }
  bool isDirectIndexedX() const;
  bool isDirectIndexedY() const;
  bool isDirectIndexedXY() const {
    return isDirectIndexedX() || isDirectIndexedY();
  }

  bool isDirectOffsetIndexed() const {
    return Kind == OperandKind::DirectOffsetIndexed;
  }

  bool isDirectOffsetIndexedX() const;
  bool isDirectOffsetIndexedY() const;
  bool isDirectOffsetIndexedSP() const;
  bool isDirectOffsetIndexedXY() const {
    return isDirectOffsetIndexedX() || isDirectOffsetIndexedY();
  }

  bool isIndirectMem() const { return Kind == OperandKind::IndirectMem; }
  bool isIndirectIndexed() const {
    return Kind == OperandKind::IndirectIndexed;
  }

  unsigned getReg() const override;
  uint64_t getImmValue() const;
  StringRef getToken() const { return Token; }

  SMLoc getStartLoc() const override { return SrcLoc.Start; }
  SMLoc getEndLoc() const override { return SrcLoc.End; }

  void print(raw_ostream &OS) const override;

  /**
   * @brief Adds the operand to an MCInst instance.
   *
   * This method may be called only on register operands. It is used by
   * auto-generated implementation of STM8AsmParser.
   */
  void addRegOperands(MCInst &Inst, unsigned N) const;

  /**
   * @brief Adds the operand to an MCInst instance.
   *
   * This method may be called only on immediate operands. It is used by
   * auto-generated implementation of STM8AsmParser.
   */
  void addImmOperands(MCInst &Inst, unsigned N) const;
  void addDirectMemOperands(MCInst &Inst, unsigned N) const;

  void addDirectIndexedOperands(MCInst &Inst, unsigned N) const;
  void addDirectIndexedXOperands(MCInst &Inst, unsigned N) const {
    addDirectIndexedOperands(Inst, N);
  }
  void addDirectIndexedYOperands(MCInst &Inst, unsigned N) const {
    addDirectIndexedOperands(Inst, N);
  }
  void addDirectIndexedXYOperands(MCInst &Inst, unsigned N) const {
    addDirectIndexedOperands(Inst, N);
  }

  void addDirectOffsetIndexedOperands(MCInst &Inst, unsigned N) const;
  void addDirectOffsetIndexedXOperands(MCInst &Inst, unsigned N) const {
    addDirectOffsetIndexedOperands(Inst, N);
  }
  void addDirectOffsetIndexedYOperands(MCInst &Inst, unsigned N) const {
    addDirectOffsetIndexedOperands(Inst, N);
  }
  void addDirectOffsetIndexedSPOperands(MCInst &Inst, unsigned N) const {
    addDirectOffsetIndexedOperands(Inst, N);
  }
  void addDirectOffsetIndexedXYOperands(MCInst &Inst, unsigned N) const {
    addDirectOffsetIndexedOperands(Inst, N);
  }

  void addIndirectMemOperands(MCInst &Inst, unsigned N) const;

  static std::unique_ptr<STM8Operand> CreateToken(const StringRef &Token,
                                                  const SMLoc &Loc);

  static std::unique_ptr<STM8Operand> CreateReg(unsigned RegNo,
                                                const SMRange &SrcLoc);
  static std::unique_ptr<STM8Operand> CreateImm(int64_t Value,
                                                const SMRange &SrcLoc);
  static std::unique_ptr<STM8Operand>
  CreateSymbolRef(MCContext &Ctx, MCSymbol *Sym, const SMRange &SrcLoc);
  static std::unique_ptr<STM8Operand> CreateDirectMem(int64_t Addr,
                                                      const SMRange &SrcLoc);
  static std::unique_ptr<STM8Operand>
  CreateDirectIndexed(unsigned RegNo, const SMRange &SrcLoc);
  static std::unique_ptr<STM8Operand>
  CreateDirectOffsetIndexed(int64_t Offset, unsigned RegNo,
                            const SMRange &SrcLoc);
  static std::unique_ptr<STM8Operand> CreateIndirectMem(int64_t PtrAddr,
                                                        const SMRange &SrcLoc);
  static std::unique_ptr<STM8Operand>
  CreateIndirectIndexed(int64_t PtrOffset, unsigned RegNo,
                        const SMRange &SrcLoc);

  static std::unique_ptr<STM8Operand> parseImmediate(MCAsmLexer &Lexer);
  static std::unique_ptr<STM8Operand> parseSymbolExpr(MCContext &Ctx,
                                                      MCAsmLexer &Lexer);
  static std::unique_ptr<STM8Operand> parseRegister(MCAsmLexer &Lexer);
  static std::unique_ptr<STM8Operand> parseDirectMemOperand(MCAsmLexer &Lexer);

  /**
   * @brief Parse directed indexed operand with optional offset.
   *
   * @param AllowedRegs List of allowed registers. If empty, all registers are
   * accepted.
   * @param Lexer Lexer to use.
   * @param WithOffset Controls if offset is required.
   */
  static std::unique_ptr<STM8Operand>
  parseDirectIndexedOperand(const ArrayRef<const unsigned> &AllowedRegs,
                            MCAsmLexer &Lexer, bool WithOffset);

  static std::unique_ptr<STM8Operand>
  parseIndirectMemOperand(MCAsmLexer &Lexer);
  static std::unique_ptr<STM8Operand>
  parseIndirectIndexedOperand(MCAsmLexer &Lexer);

  /**
   * @brief Parse an integer.
   *
   * @return True if the integer was successfully parsed, false otherwise.
   */
  static bool parseInteger(MCAsmLexer &Lexer, int64_t &Value,
                           SmallVectorImpl<AsmToken> *SavedTokens,
                           SMRange *SrcLoc);

private:
  OperandKind Kind;
  union {
    struct {
      int64_t Value;
    } Immediate;
    struct {
      unsigned RegNo;
    } Register;
    struct {
      int64_t Address;
      const MCExpr *Expr;
    } DirectMem;
    struct {
      unsigned RegNo;
    } DirectIndexed;
    struct {
      int64_t Offset;
      unsigned RegNo;
    } DirectOffsetIndexed;
    struct {
      int64_t PtrAddr;
    } IndirectMem;
    struct {
      int64_t PtrOffset;
      unsigned RegNo;
    } IndirectIndexed;
  } Oper;
  StringRef Token;
  SMRange SrcLoc;
};

} // namespace llvm

#endif // LLVM_TARGET_STM8_OPERAND_H_INCLUDED
