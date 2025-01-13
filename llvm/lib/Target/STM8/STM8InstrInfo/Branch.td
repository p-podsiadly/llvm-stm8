//==============================================================================
// Select pseudo-instructions
//==============================================================================

class TnzSelect<STM8RegClass InRC, RegisterClass OutRC>
    : STM8Pseudo<"tnz_select\t$res,$inval,$tval,$fval,$cc_flag",
        (ins InRC:$inval, OutRC:$tval, OutRC:$fval, Imm8:$cc_flag),
        (outs OutRC:$res)>;

class CompareSelect<STM8RegClass LHS, DAGOperand RHS, RegisterClass OutRC>
    : STM8Pseudo<"cp_select\t$res,$lhs,$rhs,$tval,$fval,$cc_flag",
        (ins LHS:$lhs, RHS:$rhs, OutRC:$tval, OutRC:$fval, Imm8:$cc_flag),
        (outs OutRC:$res)>;

let Uses = [CCn, CCz], usesCustomInserter = true in {
    def TnzSelect8r  : TnzSelect<AB, AnyReg>;
    def TnzSelect16r : TnzSelect<XY, AnyReg>;
}

let Uses = [CCv, CCn, CCz, CCc], usesCustomInserter = true in {
    def CompareSelect8ai8   : CompareSelect<AREG, Imm8, AnyReg>;
    def CompareSelect8ab    : CompareSelect<AREG, BREG, AnyReg>;
    def CompareSelect16ri16 : CompareSelect<XY, Imm16, AnyReg>;
    def CompareSelect16xz   : CompareSelect<XREG, ZREG, AnyReg>;
}

def : Pat<(i8 (STM8TnzSelectCC AB:$inval, AREG:$tval, AREG:$fval, timm:$cc_flag)),
          (TnzSelect8r AB:$inval, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;
def : Pat<(i16 (STM8TnzSelectCC AB:$inval, XYZ:$tval, XYZ:$fval, timm:$cc_flag)),
          (TnzSelect8r AB:$inval, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;

def : Pat<(i8 (STM8TnzSelectCC XY:$inval, AREG:$tval, AREG:$fval, timm:$cc_flag)),
          (TnzSelect16r XY:$inval, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;
def : Pat<(i16 (STM8TnzSelectCC XY:$inval, XYZ:$tval, XYZ:$fval, timm:$cc_flag)),
          (TnzSelect16r XY:$inval, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;

def : Pat<(i8 (STM8CompareSelectCC AREG:$lhs, timm:$rhs, AB:$tval, AB:$fval, timm:$cc_flag)),
          (CompareSelect8ai8 AREG:$lhs, Imm8:$rhs, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;
def : Pat<(i16 (STM8CompareSelectCC AREG:$lhs, timm:$rhs, XYZ:$tval, XYZ:$fval, timm:$cc_flag)),
          (CompareSelect8ai8 AREG:$lhs, Imm8:$rhs, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;

def : Pat<(i8 (STM8CompareSelectCC AREG:$lhs, BREG:$rhs, AB:$tval, AB:$fval, timm:$cc_flag)),
          (CompareSelect8ab AREG:$lhs, BREG:$rhs, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;
def : Pat<(i16 (STM8CompareSelectCC AREG:$lhs, BREG:$rhs, XYZ:$tval, XYZ:$fval, timm:$cc_flag)),
          (CompareSelect8ab AREG:$lhs, BREG:$rhs, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;

def : Pat<(i8 (STM8CompareSelectCC XY:$lhs, timm:$rhs, AB:$tval, AB:$fval, timm:$cc_flag)),
          (CompareSelect16ri16 XY:$lhs, Imm16:$rhs, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;
def : Pat<(i16 (STM8CompareSelectCC XY:$lhs, timm:$rhs, XYZ:$tval, XYZ:$fval, timm:$cc_flag)),
          (CompareSelect16ri16 XY:$lhs, Imm16:$rhs, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;

def : Pat<(i8 (STM8CompareSelectCC XREG:$lhs, ZREG:$rhs, AB:$tval, AB:$fval, timm:$cc_flag)),
          (CompareSelect16xz XREG:$lhs, ZREG:$rhs, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;
def : Pat<(i16 (STM8CompareSelectCC XREG:$lhs, ZREG:$rhs, XYZ:$tval, XYZ:$fval, timm:$cc_flag)),
          (CompareSelect16xz XREG:$lhs, ZREG:$rhs, AnyReg:$tval, AnyReg:$fval, Imm8:$cc_flag)>;

// Special case for patterns which are results of selecting ISD::SETCC.
def : Pat<(i8 (trunc (STM8TnzSelectCC AB:$inval, (i16 1), (i16 0), timm:$cc_flag))),
          (TnzSelect8r AB:$inval, (LDai8 1), (CLRa), Imm8:$cc_flag)>;
def : Pat<(i8 (trunc (STM8CompareSelectCC AREG:$lhs, timm:$rhs, (i16 1), (i16 0), timm:$cc_flag))),
          (CompareSelect8ai8 AREG:$lhs, Imm8:$rhs, (LDai8 1), (CLRa), Imm8:$cc_flag)>;
def : Pat<(i8 (trunc (STM8CompareSelectCC AREG:$lhs, BREG:$rhs, (i16 1), (i16 0), timm:$cc_flag))),
          (CompareSelect8ab AREG:$lhs, BREG:$rhs, (LDai8 1), (CLRa), Imm8:$cc_flag)>;
def : Pat<(i8 (trunc (STM8CompareSelectCC XY:$lhs, timm:$rhs, (i16 1), (i16 0), timm:$cc_flag))),
          (CompareSelect16ri16 XY:$lhs, Imm16:$rhs, (LDai8 1), (CLRa), Imm8:$cc_flag)>;

//==============================================================================
// Jump/Call instruction formats
//==============================================================================

multiclass JumpCall_Fmt<string mnemonic, bits<4> baseOpCode> {
    def m16 : STM8Inst<mnemonic # "\t$addr", (ins DirectMem16:$addr), (outs), []> {
        bits<16> addr;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 20} = 0xC;
        let Inst{19 - 16} = baseOpCode;
        let Inst{15 - 0} = addr;
    }

    let isIndirectBranch = true in {
        def ix : STM8Inst<mnemonic # "\t$addr", (ins DirectIdxX:$addr), (outs), []> {
            let Size = 1;
            field bits<8> Inst;
            let Inst{7 - 4} = 0xF;
            let Inst{3 - 0} = baseOpCode;
        }

        def ixd8 : STM8Inst<mnemonic # "\t$addr", (ins DirectOff8IdxX:$addr), (outs), []> {
            bits<8> addr;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 12} = 0xE;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = addr;
        }

        def ixd16 : STM8Inst<mnemonic # "\t$addr", (ins DirectOff16IdxX:$addr), (outs), []> {
            bits<16> addr;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 20} = 0xD;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = addr;
        }

        def iy : STM8Inst<mnemonic # "\t$addr", (ins DirectIdxY:$addr), (outs), []> {
            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 8} = 0x90;
            let Inst{7 - 4} = 0xF;
            let Inst{3 - 0} = baseOpCode;
        }

        def iyd8 : STM8Inst<mnemonic # "\t$addr", (ins DirectOff8IdxY:$addr), (outs), []> {
            bits<8> addr;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 16} = 0x90;
            let Inst{15 - 12} = 0xE;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = addr;
        }

        def iyd16 : STM8Inst<mnemonic # "\t$addr", (ins DirectOff16IdxY:$addr), (outs), []> {
            bits<16> addr;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 24} = 0x90;
            let Inst{23 - 20} = 0xD;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = addr;
        }

        // TODO add instruction for indirect short mem.
        def im16 : STM8Inst<mnemonic # "\t$addr", (ins IndirectMem16:$addr), (outs), []> {
            bits<16> addr;
            
            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 24} = 0x72;
            let Inst{23 - 20} = 0xC;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = addr;
        }
    }
}

class JumpRel_Fmt<string mnemonic, bits<8> preOpCode, bits<4> baseOpCode, list<Register> usedRegs>
    : STM8Inst<mnemonic # "\t$target", (ins PCRel8:$target), (outs)> {
    bits<8> target;
    bit is3Byte = !ne(preOpCode, 0x0);

    let Size = !cond(is3Byte: 3, true: 2);
    field bits<24> Inst;
    let Inst{23 - 16} = preOpCode;
    let Inst{15 - 12} = 0x2;
    let Inst{11 - 8} = baseOpCode;
    let Inst{7 - 0} = target;

    let Uses = usedRegs;
}

//==============================================================================
// Jumps
//==============================================================================

let isBranch = true, isTerminator = true in {
    let isBarrier = true in {
        defm JP : JumpCall_Fmt<"jp", 0xC>;

        def GenJPir : STM8GenInst<"jp\t$target", (ins DirectIdxX:$target), (outs)> {
            let isIndirectBranch = true;
        }

        def JRA   : JumpRel_Fmt<"jra ",  0x0,  0x0, []>; // Jump always
    }

    def JRF   : JumpRel_Fmt<"jrf ",  0x0,  0x1, []>; // Jump if false
    def JRIH  : JumpRel_Fmt<"jrih",  0x90, 0xF, []>; // Jump if interrupt line is high
    def JRIL  : JumpRel_Fmt<"jril",  0x90, 0xE, []>; // Jump if interrupt line is low

    def JRC   : JumpRel_Fmt<"jrc ",  0x0,  0x5, [CCc]>;           // Jump if C = 1
    def JREQ  : JumpRel_Fmt<"jreq",  0x0,  0x7, [CCz]>;           // Jump if Z = 1
    def JRH   : JumpRel_Fmt<"jrh ",  0x90, 0x9, [CCh]>;           // Jump if H = 1
    def JRM   : JumpRel_Fmt<"jrm ",  0x90, 0xD, [CCi]>;           // Jump if I = 1
    def JRMI  : JumpRel_Fmt<"jrmi",  0x0,  0xB, [CCn]>;           // Jump if N = 1
    def JRNC  : JumpRel_Fmt<"jrnc",  0x0,  0x4, [CCc]>;           // Jump if C = 0
    def JRNE  : JumpRel_Fmt<"jrne",  0x0,  0x6, [CCz]>;           // Jump if Z = 0
    def JRNH  : JumpRel_Fmt<"jrnh",  0x90, 0x8, [CCh]>;           // Jump if H = 0
    def JRNM  : JumpRel_Fmt<"jrnm",  0x90, 0xC, [CCi]>;           // Jump if I = 0
    def JRNV  : JumpRel_Fmt<"jrnv",  0x0,  0x8, [CCv]>;           // Jump if V = 0
    def JRPL  : JumpRel_Fmt<"jrpl",  0x0,  0xA, [CCn]>;           // Jump if N = 0
    def JRSGE : JumpRel_Fmt<"jrsge", 0x0,  0xE, [CCv, CCn]>;      // Jump if (N XOR V) = 0
    def JRSGT : JumpRel_Fmt<"jrsgt", 0x0,  0xC, [CCv, CCn, CCz]>; // Jump if (Z OR (N XOR V)) = 0
    def JRSLE : JumpRel_Fmt<"jrsle", 0x0,  0xD, [CCv, CCn, CCz]>; // Jump if (Z OR (N XOR V)) = 1
    def JRSLT : JumpRel_Fmt<"jrslt", 0x0,  0xF, [CCv, CCn]>;      // Jump if (N XOR V) = 1
    def JRUGT : JumpRel_Fmt<"jrugt", 0x0,  0x2, [CCz, CCc]>;      // Jump if C = 0 and Z = 0
    def JRULE : JumpRel_Fmt<"jrule", 0x0,  0x3, [CCz, CCc]>;      // Jump if C = 1 or Z = 1
    def JRV   : JumpRel_Fmt<"jrv ",  0x0,  0x9, [CCv]>;           // Jump if V = 1
}

def : MnemonicAlias<"jrt",   "jra">;  // Jump if true
def : MnemonicAlias<"jruge", "jrnc">; // Jump if unsigned greater equal
def : MnemonicAlias<"jrult", "jrc">;  // Jump if unsigned less than

def : Pat<(STM8Wrapper tjumptable:$jt), (GenLDWri16 tjumptable:$jt)>;
def : Pat<(add i16:$lhs, (STM8Wrapper tjumptable:$jt)), (GenADDWri16 XY:$lhs, tjumptable:$jt)>;
def : Pat<(brind i16:$target), (GenJPir IR16:$target)>;

def : Pat<(br bb:$target), (JRA PCRel8:$target)>;

//==============================================================================
// Fused Compare/Test-Jump pseudo-instructions for use before reg alloc
//==============================================================================

let isBranch = true, isTerminator = true, Defs = [CCn, CCz] in {
    def TNZr_JR : STM8Pseudo<"tnz_jr\t$in,$cc,$target",
        (ins AB:$in, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8TnzJump i8:$in, timm:$cc, bb:$target)]>;

    def TNZispd8_JR : STM8Pseudo<"tnz_jr\t$in,$cc,$target",
        (ins DirectOff8SP:$in, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8TnzJump SPDispCP:$in, timm:$cc, bb:$target)]>;

    def TNZWr_JR : STM8Pseudo<"tnzw_jr\t$in,$cc,$target",
        (ins XY:$in, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8TnzJump i16:$in, timm:$cc, bb:$target)]>;
}

let isBranch = true, isTerminator = true, Defs = [CCv, CCn, CCz, CCc] in {
    def CPai8_JR : STM8Pseudo<"cp_jr\t$lhs,$rhs,$cc,$target",
        (ins AREG:$lhs, Imm8:$rhs, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8CompareJump i8:$lhs, timm:$rhs, timm:$cc, bb:$target)]>;

    def CPab_JR : STM8Pseudo<"cp_jr\t$lhs,$rhs,$cc,$target",
        (ins AREG:$lhs, BREG:$rhs, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8CompareJump i8:$lhs, i8:$rhs, timm:$cc, bb:$target)]>;

    def CPaispd8_JR : STM8Pseudo<"cp_jr\t$lhs,$rhs,$cc,$target",
        (ins AREG:$lhs, DirectOff8SP:$rhs, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8CompareJump i8:$lhs, SPDispCP:$rhs, timm:$cc, bb:$target)]>;

    def CPWri16_JR : STM8Pseudo<"cp_jr\t$lhs,$rhs,$cc,$target",
        (ins XY:$lhs, Imm16:$rhs, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8CompareJump i16:$lhs, timm:$rhs, timm:$cc, bb:$target)]>;

    def CPWxz_JR : STM8Pseudo<"cp_jr\t$lhs,$rhs,$cc,$target",
        (ins XREG:$lhs, ZREG:$rhs, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8CompareJump i16:$lhs, i16:$rhs, timm:$cc, bb:$target)]>;

    def CPWxispd8_JR : STM8Pseudo<"cp_jr\t$lhs,$rhs,$cc,$target",
        (ins XREG:$lhs, DirectOff8SP:$rhs, Imm8:$cc, PCRel8:$target), (outs),
        [(STM8CompareJump i16:$lhs, SPDispCP:$rhs, timm:$cc, bb:$target)]>;
}

//==============================================================================
// Calls
//==============================================================================

def SDT_STM8CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_STM8CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_STM8Call         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;

def STM8Call : SDNode<"STM8::CALL", SDT_STM8Call,
    [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

def STM8CallSeqStart : SDNode<"ISD::CALLSEQ_START", SDT_STM8CallSeqStart,
                            [SDNPHasChain, SDNPOutGlue]>;

def STM8CallSeqEnd   : SDNode<"ISD::CALLSEQ_END", SDT_STM8CallSeqEnd,
                            [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

let Defs = [SP], Uses = [SP] in {

    def ADJCALLSTACKDOWN : STM8Pseudo<"ADJCALLSTACKDOWN", (ins Imm16:$amt1, Imm16:$amt2), (outs),
                [(STM8CallSeqStart timm:$amt1, timm:$amt2)]>;

    def ADJCALLSTACKUP : STM8Pseudo<"ADJCALLSTACKUP", (ins Imm16:$amt1, Imm16:$amt2), (outs),
                [(STM8CallSeqEnd timm:$amt1, timm:$amt2)]>;

} // Defs

let isCall = true, Uses = [SP], Defs = [SP] in {
    defm CALL : JumpCall_Fmt<"call", 0xD>;
}

def : Pat<(STM8Call Imm16Pat:$addr), (CALLm16 DirectMem16:$addr)>;
def : Pat<(STM8Call tglobaladdr:$addr), (CALLm16 tglobaladdr:$addr)>;
def : Pat<(STM8Call texternalsym:$addr), (CALLm16 texternalsym:$addr)>;

//==============================================================================
// Interrupt instruction, only used in the ISR vector
//==============================================================================

def INTm16 : STM8Inst<"int\t$addr", (ins DirectMem16:$addr), (outs)> {
    bits<16> addr;

    let Size = 4;
    field bits<32> Inst;
    let Inst{31 - 16} = 0x8200;
    let Inst{15 - 0} = addr;
}
