#include "InputFiles.h"
#include "Symbols.h"
#include "Target.h"
#include "Thunks.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

#define DEBUG_TYPE "stm8"

namespace {

class STM8 final : public TargetInfo {
public:
  STM8() {
    defaultCommonPageSize = 256;
    defaultMaxPageSize = 256;
  }

  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;

  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;

  int64_t getImplicitAddend(const uint8_t *buf, RelType type) const override;
};

} // namespace

RelExpr STM8::getRelExpr(RelType type, const Symbol &S,
                         const uint8_t *loc) const {
  switch (type) {
  case R_STM8_8:
  case R_STM8_16:
    return R_ABS;
  case R_STM8_8_PCREL:
    return R_PC;
  default:
    break;
  }

  error(getErrorLocation(loc) + "unknown relocation (" + Twine(type) +
        ") against symbol " + toString(S));
  return R_NONE;
}

void STM8::relocate(uint8_t *loc, const Relocation &rel, uint64_t val) const {
  switch (rel.type) {
  case R_STM8_8: {
    LLVM_DEBUG(dbgs() << "Applying R_STM8_8 reloc");
    checkUInt(loc, val, 8, rel);
    *loc = val;
    break;
  }
  case R_STM8_16: {
    LLVM_DEBUG(dbgs() << "Applying R_STM8_16 reloc");
    checkUInt(loc, val, 16, rel);
    write16be(loc, val);
    break;
  }
  case R_STM8_8_PCREL: {
    LLVM_DEBUG(dbgs() << "Applying R_STM8_8_PCREL reloc");
    checkUInt(loc, val, 8, rel);
    *loc = val;
    break;
  }
  default:
    llvm_unreachable("unknown relocation");
  }
}

int64_t STM8::getImplicitAddend(const uint8_t *buf, RelType type) const {
  switch (type) {
  default:
    internalLinkerError(getErrorLocation(buf),
                        "cannot read addend for relocation " + toString(type));
    break;
  case R_STM8_8:
    return static_cast<int64_t>(buf[0]);
  case R_STM8_16:
    return read16be(buf);
  case R_STM8_8_PCREL:
    return -static_cast<int64_t>(buf[0]);
  };

  return 0;
}

TargetInfo *elf::getSTM8TargetInfo() {
  static STM8 target;
  return &target;
}
