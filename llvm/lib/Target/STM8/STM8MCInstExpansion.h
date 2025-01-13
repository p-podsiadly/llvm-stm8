#ifndef LLVM_STM8_MC_INST_EXPANSION_H
#define LLVM_STM8_MC_INST_EXPANSION_H

#include <llvm/ADT/SmallVector.h>

namespace llvm {

class MCInst;

using ExpandedMCInst = SmallVector<MCInst, 4>;

ExpandedMCInst ExpandSTM8Inst(const MCInst &Inst);

} // namespace llvm

#endif // LLVM_STM8_MC_INST_EXPANSION_H
