#ifndef LLVM_STM8_PASSES_H
#define LLVM_STM8_PASSES_H

namespace llvm {

class PassRegistry;
class MachineFunctionPass;

MachineFunctionPass *createSTM8EarlyImagRegLoweringPass();
void initializeSTM8EarlyImagRegLoweringPass(PassRegistry &);

MachineFunctionPass *createSTM8LowerGenLdStPass();
void initializeSTM8LowerGenLdStPass(PassRegistry &);

MachineFunctionPass *createSTM8LateInstrEliminationPass();
void initializeSTM8LateInstrEliminationPass(PassRegistry &);

}

#endif // LLVM_STM8_PASSES_H
