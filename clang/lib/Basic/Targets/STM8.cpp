#include "STM8.h"
#include "clang/Basic/MacroBuilder.h"

namespace clang {
namespace targets {

STM8TargetInfo::STM8TargetInfo(const llvm::Triple &Triple,
                               const TargetOptions &)
    : TargetInfo(Triple) {
  TLSSupported = false;
  PointerWidth = 16;
  PointerAlign = 8;
  IntWidth = 16;
  IntAlign = 8;
  LongWidth = 32;
  LongAlign = 8;
  LongLongWidth = 64;
  LongLongAlign = 8;
  SuitableAlign = 8;
  DefaultAlignForAttributeAligned = 8;
  HalfWidth = 16;
  HalfAlign = 8;
  FloatWidth = 32;
  FloatAlign = 8;
  DoubleWidth = 32;
  DoubleAlign = 8;
  DoubleFormat = &llvm::APFloat::IEEEsingle();
  LongDoubleWidth = 32;
  LongDoubleAlign = 8;
  LongDoubleFormat = &llvm::APFloat::IEEEsingle();
  SizeType = UnsignedInt;
  PtrDiffType = SignedInt;
  IntPtrType = SignedInt;
  Char16Type = UnsignedInt;
  WIntType = SignedInt;
  Int16Type = SignedInt;
  Char32Type = UnsignedLong;
  SigAtomicType = SignedChar;

  resetDataLayout("E-P0-p:16:8-i8:8-i16:8-i32:8-i64:8-f32:8-f64:8-n8:16-a:8");
}

void STM8TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  Builder.defineMacro("STM8");
  Builder.defineMacro("__STM8");
  Builder.defineMacro("__STM8__");
}

ArrayRef<Builtin::Info> STM8TargetInfo::getTargetBuiltins() const { return {}; }

TargetInfo::BuiltinVaListKind STM8TargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::VoidPtrBuiltinVaList;
}

bool STM8TargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &info) const {
  // TODO
  ((void)Name);
  ((void)info);
  return false;
}

std::string_view STM8TargetInfo::getClobbers() const {
  // TODO
  return {};
}

TargetInfo::CallingConvCheckResult
STM8TargetInfo::checkCallingConvention(CallingConv CC) const {
  switch (CC) {
  case CC_C:
  case CC_SDCCCallV0:
  case CC_SDCCCallV1:
    return CCCR_OK;
  default:
    break;
  }

  return TargetInfo::checkCallingConvention(CC);
}

ArrayRef<const char *> STM8TargetInfo::getGCCRegNames() const {
  // TODO
  return {};
}

ArrayRef<TargetInfo::GCCRegAlias> STM8TargetInfo::getGCCRegAliases() const {
  // TODO
  return {};
}

} // namespace targets
} // namespace clang
