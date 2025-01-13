#ifndef LLVM_STM8_DISASSEMBLER_H
#define LLVM_STM8_DISASSEMBLER_H

#include "llvm/MC/MCDisassembler/MCDisassembler.h"

namespace llvm {

class STM8Disassembler : public MCDisassembler {
public:
  STM8Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx);

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

} // namespace llvm

#endif // LLVM_STM8_DISASSEMBLER_H
