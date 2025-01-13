#ifndef LLVM_STM8_ASM_PRINTER_H_INCLUDED
#define LLVM_STM8_ASM_PRINTER_H_INCLUDED

#include "llvm/CodeGen/AsmPrinter.h"

namespace llvm {

class STM8AsmPrinter : public AsmPrinter {
public:
  STM8AsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer);

  StringRef getPassName() const override;

  void emitInstruction(const MachineInstr *MI) override;

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  /// Emit interrupt service routine section for functions marked with
  /// "interrupt" attribute.
  void emitISRSection(int IVIndex, MachineFunction &MF);

  void emitUnbundledInstruction(const MachineInstr *MI);

  bool lowerOperand(const MachineOperand &MO, MCOperand &MCO) const;
  bool lowerSymbol(const MCSymbol *Sym, MCOperand &MCO) const;

  void emitPseudoInst(MCInst &Inst);

  StringRef getInstName(unsigned Opcode) const;
};

} // namespace llvm

#endif // LLVM_STM8_ASM_PRINTER_H_INCLUDED
