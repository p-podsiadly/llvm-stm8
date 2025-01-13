#include "STM8TargetMachine.h"

#include "Passes/STM8Passes.h"
#include "STM8DAGToDAGISel.h"
#include "STM8MachineFunctionInfo.h"
#include "TargetInfo/STM8TargetInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

namespace llvm {

namespace {

/**
 * @brief String specifying data layout to use for the code.
 *
 * * "E": big endian
 * * "P0": Code and data in the same address space.
 * * "p:16:8": Size and alignment of pointers: 16 bits, 8 bit alignment.
 * * "i8:8", "i16:8", "i32:8", "i64:8", "f32:8": all types require 8 bit
 * alignment.
 * * "n8:16": native integer types are i8 and i16.
 * " "a:8": 8 bit alignment of aggregates.
 *
 * @see https://releases.llvm.org/17.0.1/docs/LangRef.html#data-layout
 */
const char *STM8DataLayout =
    "E-P0-p:16:8-i8:8-i16:8-i32:8-i64:8-f32:8-f64:8-n8:16-a:8";

// All STM8 models have the same CPU.
const char *STM8CPUName = "generic";

Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

CodeModel::Model getEffectiveSTM8CodeModel(std::optional<CodeModel::Model> CM) {
  if (CM) {
    if (*CM != CodeModel::Tiny) {
      report_fatal_error("Target only supports the tiny CodeModel", false);
    }

    return *CM;
  }

  return CodeModel::Tiny;
}

class STM8PassConfig : public TargetPassConfig {
public:
  STM8PassConfig(STM8TargetMachine &targetMachine, PassManagerBase &passMgr)
      : TargetPassConfig(targetMachine, passMgr) {}

  STM8TargetMachine &getSTM8TargetMachine() const {
    return getTM<STM8TargetMachine>();
  }

  /**
   * @brief Registers an instruction selector pass.
   *
   * @return @b true if an error ocurred, @b false on success!
   * @see TargetPassConfig::addISelPasses()
   */
  bool addInstSelector() override {
    addPass(new STM8DAGToDAGISel(getSTM8TargetMachine(), getOptLevel()));

    // False == everything's fine.
    return false;
  }

  void addMachinePasses() override {
    addPass(createSTM8EarlyImagRegLoweringPass());
    TargetPassConfig::addMachinePasses();
  }

  void addPreRegAlloc() override { addPass(createSTM8LowerGenLdStPass()); }

  void addPreEmitPass() override {
    addPass(createSTM8LateInstrEliminationPass());
  }

  /// Always force optimized reg alloc. Optimized RA allows us to insert imag
  /// register lowering pass between register assignment and rewriting. This is
  /// important as lowering has to happen before any registers are spilled.
  void addFastRegAlloc() override { addOptimizedRegAlloc(); }
};

} // namespace

STM8TargetMachine::STM8TargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOptLevel OL, bool JIT)
    : LLVMTargetMachine(T, STM8DataLayout, TT, STM8CPUName, FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveSTM8CodeModel(CM), OL),
      SubTarget(TT, STM8CPUName, FS, *this) {
  TLOF = std::make_unique<TargetLoweringObjectFileELF>();

  // Creates various STM8-specific MC objects previously registered in LLVM.
  initAsmInfo();
}

STM8TargetMachine::~STM8TargetMachine() = default;

TargetLoweringObjectFile *STM8TargetMachine::getObjFileLowering() const {
  return TLOF.get();
}

const STM8Subtarget *
STM8TargetMachine::getSubtargetImpl(const Function &) const {
  return &SubTarget;
}

TargetPassConfig *
STM8TargetMachine::createPassConfig(PassManagerBase &passMgr) {
  return new STM8PassConfig(*this, passMgr);
}

MachineFunctionInfo *STM8TargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return MachineFunctionInfo::create<STM8MachineFunctionInfo>(Allocator, F,
                                                              STI);
}

} // namespace llvm

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSTM8Target() {
  RegisterTargetMachine<STM8TargetMachine> X(getTheSTM8Target());

  auto &PR = *PassRegistry::getPassRegistry();
  initializeSTM8DAGToDAGISelPass(PR);
  initializeSTM8EarlyImagRegLoweringPass(PR);
  initializeSTM8LowerGenLdStPass(PR);
  initializeSTM8LateInstrEliminationPass(PR);
}
