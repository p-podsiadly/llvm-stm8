#ifndef LLVM_STM8_MC_CODE_EMITTER_H_INCLUDED
#define LLVM_STM8_MC_CODE_EMITTER_H_INCLUDED

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {

class MCInstrInfo;
class MCContext;
class MCOperand;

class STM8MCCodeEmitter : public MCCodeEmitter {
public:
  STM8MCCodeEmitter(const MCInstrInfo &InstrInfo, MCContext &Ctx);

  // Implementation of MCCodeEmitter methods

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

private:
  const MCInstrInfo &InstrInfo;
  MCContext &Ctx;

  /// Returns the binary encoding of operand.
  ///
  /// If the machine operand requires relocation, the relocation is recorded
  /// and zero is returned.
  uint64_t getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  uint64_t encodeImm8(const MCInst &MI, unsigned OpIdx,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const;

  uint64_t encodeImm16(const MCInst &MI, unsigned OpIdx,
                       SmallVectorImpl<MCFixup> &Fixups,
                       const MCSubtargetInfo &STI) const;

  uint64_t encodeDirectMem(const MCInst &MI, unsigned OpIdx,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &STI) const;

  uint64_t encodeIndirectMem(const MCInst &MI, unsigned OpIdx,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  uint64_t encodePCRelImm(const MCInst &MI, unsigned OpIdx,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  uint64_t encodeImmWithPossibleFixup(const MCInst &MI, unsigned OpIdx,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI,
                                      unsigned FixupKind, unsigned FixupBits,
                                      unsigned Addend = 0) const;

  uint64_t encodeOperandDOI(const MCInst &MI, unsigned OpIdx,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;

  uint32_t getOperandBitOffset(const MCInst &MI, unsigned OpNum,
                               const MCSubtargetInfo &STI) const;
};

} // namespace llvm

#endif // LLVM_STM8_MC_CODE_EMITTER_H_INCLUDED
