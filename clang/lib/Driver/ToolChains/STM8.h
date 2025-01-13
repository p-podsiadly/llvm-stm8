#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_STM8_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_STM8_H

#include "Gnu.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY STM8ToolChain : public Generic_ELF {
public:
  STM8ToolChain(const Driver &D, const llvm::Triple &Triple,
                const llvm::opt::ArgList &Args);

  std::string getCompilerRT(const llvm::opt::ArgList &Args, StringRef Component,
                            FileType Type) const override;

  bool HasNativeLLVMSupport() const override { return true; }

protected:
  Tool *buildLinker() const override;
};

} // namespace toolchains
} // namespace driver
} // namespace clang

namespace clang {
namespace driver {
namespace tools {
namespace STM8 {

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  explicit Linker(const ToolChain &TC) : Tool("stm8::Linker", "ld.lld", TC) {}

  bool isLinkJob() const override { return true; }
  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &Args,
                    const char *LinkingOutput) const override;
};

} // namespace STM8
} // namespace tools
} // namespace driver
} // namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_STM8_H
