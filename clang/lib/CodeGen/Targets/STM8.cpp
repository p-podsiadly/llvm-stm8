//===- STM8.cpp -----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"
#include "clang/Basic/DiagnosticFrontend.h"

using namespace clang;
using namespace clang::CodeGen;

namespace {

class STM8ABIInfo : public DefaultABIInfo {
public:
  STM8ABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}
};

class STM8TargetCodeGenInfo : public TargetCodeGenInfo {
public:
  STM8TargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<STM8ABIInfo>(CGT)) {}

  void setTargetAttributes(const Decl *D, llvm::GlobalValue *GV,
                           CodeGen::CodeGenModule &M) const override {
    const auto *FuncDecl = dyn_cast_or_null<FunctionDecl>(D);
    if (!FuncDecl || GV->isDeclaration()) {
      return;
    }

    auto *Fn = cast<llvm::Function>(GV);
    if (auto Attr = FuncDecl->getAttr<STM8InterruptAttr>(); Attr) {
      Fn->addFnAttr("interrupt", llvm::utostr(Attr->getInterrupt()));
    }
  }
};

} // namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createSTM8TargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<STM8TargetCodeGenInfo>(CGM.getTypes());
}
