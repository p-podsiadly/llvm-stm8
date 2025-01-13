#ifndef LLVM_STM8_CALLING_CONV_H
#define LLVM_STM8_CALLING_CONV_H

#include <llvm/CodeGen/CallingConvLower.h>

namespace llvm {
namespace STM8 {

CCAssignFn *getCCAssignFn(CallingConv::ID CallConv);
CCAssignFn *getRetCCAssignFn(CallingConv::ID CallConv);

} // namespace STM8
} // namespace llvm

#endif // LLVM_STM8_CALLING_CONV_H
