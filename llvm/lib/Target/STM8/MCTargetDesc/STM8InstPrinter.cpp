#include "STM8InstPrinter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "STM8GenAsmWriter.inc"

STM8InstPrinter::STM8InstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                                 const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {}

void STM8InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &O) {
  // TODO
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void STM8InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  const MCOperand &Operand = MI->getOperand(OpNo);

  if (Operand.isReg()) {
    O << getRegisterName(Operand.getReg());
  } else {
    // TODO
    Operand.print(O);
  }
}

void STM8InstPrinter::printImm8(const MCInst *MI, unsigned OpNo,
                                raw_ostream &O) {
  auto Val = static_cast<uint8_t>(MI->getOperand(OpNo).getImm());
  O << "#" << format_hex(Val, 0);
}

void STM8InstPrinter::printImm16(const MCInst *MI, unsigned OpNo,
                                 raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    uint16_t Val = static_cast<uint16_t>(Op.getImm());
    O << "#" << format_hex(Val, 0);
  } else if (Op.isExpr()) {
    const MCExpr *Expr = Op.getExpr();
    Expr->print(O, &MAI, /*InParens=*/false);
  } else {
    llvm_unreachable("Unexpected operand kind");
  }
}

void STM8InstPrinter::printDirectMemOperand(const MCInst *MI, unsigned OpNo,
                                            raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    uint16_t Val = static_cast<uint16_t>(Op.getImm());
    O << format_hex(Val, 0);
  } else if (Op.isExpr()) {
    const MCExpr *Expr = Op.getExpr();
    Expr->print(O, &MAI, /*InParens=*/false);
  } else {
    llvm_unreachable("Unexpected operand kind");
  }
}

void STM8InstPrinter::printDirectIndexedOperand(const MCInst *MI, unsigned OpNo,
                                                raw_ostream &O) {
  unsigned RegNo = MI->getOperand(OpNo).getReg();
  O << "(" << getRegisterName(RegNo) << ")";
}

void STM8InstPrinter::printOperandDOI(const MCInst *MI, unsigned OpNo,
                                      raw_ostream &O) {
  unsigned RegNo = MI->getOperand(OpNo).getReg();
  int64_t Offset = MI->getOperand(OpNo + 1).getImm();

  assert(RegNo != 0 && "Register number is not set!");

  O << "(#" << format_hex(Offset, 0) << "," << getRegisterName(RegNo) << ")";
}

void STM8InstPrinter::printPCRelImm(const MCInst *MI, uint64_t Address,
                                    unsigned OpNo, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isImm()) {
    int64_t Offset = MO.getImm();
    O << format_hex(Offset, 2);
  } else if (MO.isExpr()) {
    const MCExpr *Expr = MO.getExpr();
    O << *Expr;
  } else {
    llvm_unreachable("Unsupported PC Rel operand!");
  }
}

void STM8InstPrinter::printIndirectMemOperand(const MCInst *MI, unsigned OpNo,
                                              raw_ostream &O) const {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isImm()) {
    int64_t Addr = MO.getImm();
    O << "[" << format_hex(Addr, 0) << "]";
  } else if (MO.isExpr()) {
    const MCExpr *Expr = MO.getExpr();
    O << "[" << *Expr << "]";
  } else {
    llvm_unreachable("Unsupported indirect mem operand");
  }
}

} // namespace llvm
