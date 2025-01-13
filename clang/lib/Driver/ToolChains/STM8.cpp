#include "STM8.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/TargetParser/SubtargetFeature.h"

using namespace llvm::opt;
using namespace clang;
using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang::driver::toolchains;

STM8ToolChain::STM8ToolChain(const Driver &D, const llvm::Triple &Triple,
                             const llvm::opt::ArgList &Args)
    : Generic_ELF(D, Triple, Args) {}

std::string STM8ToolChain::getCompilerRT(const llvm::opt::ArgList &Args,
                                         StringRef Component,
                                         FileType Type) const {
  // TODO
  return {};
}

Tool *STM8ToolChain::buildLinker() const { return new STM8::Linker(*this); }

void STM8::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                const InputInfo &Output,
                                const InputInfoList &Inputs,
                                const llvm::opt::ArgList &Args,
                                const char *LinkingOutput) const {
  ((void)0);

  std::string Linker = getToolChain().GetProgramPath(getShortName());

  ArgStringList CmdArgs;

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  getToolChain().AddFilePathLibArgs(Args, CmdArgs);

  AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);

  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileCurCP(), Args.MakeArgString(Linker),
      CmdArgs, Inputs, Output));
}
