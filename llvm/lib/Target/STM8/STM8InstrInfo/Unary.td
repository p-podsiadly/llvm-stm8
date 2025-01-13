
class OpNodeInfo<SDPatternOperator opNode> {
    field bit IsUndef = !eq(opNode, undef);

    field bit HasIn = !cond(
        !isa<PatFrag>(opNode): !not(!empty(!cast<PatFrag>(opNode).Operands)),
        // assume that unary instructions with target-specific SDNode have inputs (SLL, SRL...)
        !isa<SDNode>(opNode): true,
        // assume unary instructions with undef pattern have inputs (RLC, RRC...)
        IsUndef: true
    );

    field string DstSrcConstraint = !cond(
        HasIn: "$dst = $src",
        true:  ""
    );
}

def IncPatFrag   : PatFrag<(ops node:$src), (add node:$src, 1)>;
def DecPatFrag   : PatFrag<(ops node:$src), (sub node:$src, 1)>;
def NegPatFrag   : PatFrag<(ops node:$src), (ineg node:$src)>;
def CPLPatFrag8  : PatFrag<(ops node:$src), (xor node:$src, 0xff)>;
def CPLPatFrag16 : PatFrag<(ops node:$src), (xor node:$src, 0xffff)>;
def ClrPatFrag8  : PatFrag<(ops),           (i8 0)>;
def ClrPatFrag16 : PatFrag<(ops),           (i16 0)>;

////////////////////////////////////////////////////////////////////////////////
// 8-bit increment/decrement
////////////////////////////////////////////////////////////////////////////////

multiclass Unary_Fmt<string mnemonic, bits<4> baseOpCode, SDPatternOperator opNode> {

    def a : STM8Inst<mnemonic # "\t$dst",
        !cond(
            OpNodeInfo<opNode>.HasIn: (ins AREG:$src),
            true:                     (ins)
        ),
        (outs AREG:$dst),
        !cond(
            OpNodeInfo<opNode>.IsUndef: [],
            OpNodeInfo<opNode>.HasIn:   [(set i8:$dst, (opNode i8:$src))],
            true:                       [(set i8:$dst, (opNode))]
        )> {
        let Constraints = OpNodeInfo<opNode>.DstSrcConstraint;
        let Size = 1;
        field bits<8> Inst;
        let Inst{7 - 4} = 0x4;
        let Inst{3 - 0} = baseOpCode;
    }
 
    let mayLoad = OpNodeInfo<opNode>.HasIn, mayStore = true in {

        def m8 : STM8Inst<mnemonic # "\t$addr",
            (ins DirectMem8:$addr), (outs),
            !cond(
                OpNodeInfo<opNode>.IsUndef: [],
                OpNodeInfo<opNode>.HasIn:   [(store (opNode (i8 (load Imm8Pat:$addr))), Imm8Pat:$addr)],
                true:                       [(store (opNode), Imm8Pat:$addr)]
            )> {
            bits<8> addr;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 12} = 0x3;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = addr;
        }

        def m16 : STM8Inst<mnemonic # "\t$addr",
            (ins DirectMem16:$addr), (outs),
            !cond(
                OpNodeInfo<opNode>.IsUndef: [],
                OpNodeInfo<opNode>.HasIn:   [(store (opNode (i8 (load Imm16Pat:$addr))), Imm16Pat:$addr)],
                true:                       [(store (opNode), Imm16Pat:$addr)]
            )> {
            bits<16> addr;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 20} = 0x725;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = addr;
        }

        foreach RC = [XREG, YREG] in {
            defvar isY = !eq(RC, YREG);

            def "i" # RC.RS : STM8Inst<mnemonic # "\t$addr",
                (ins DirectIndexed<RC>:$addr), (outs), []> {
                let Size = !add(1, isY);
                field bits<16> Inst;
                let Inst{15 - 4} = 0x907;
                let Inst{3 - 0} = baseOpCode;
            }

            def "i" # RC.RS # "d8" : STM8Inst<mnemonic # "\t$addr",
                (ins DirectOffsetIndexed<RC, Imm8>:$addr), (outs), []> {
                bits<8> addr;

                let Size = !add(2, isY);
                field bits<24> Inst;
                let Inst{23 - 12} = 0x906;
                let Inst{11 - 8} = baseOpCode;
                let Inst{7 - 0} = addr;
            }

            def "i" # RC.RS # "d16" : STM8Inst<mnemonic # "\t$addr",
                (ins DirectOffsetIndexed<RC, Imm16>:$addr), (outs), []> {
                bits<16> addr;

                let Size = 4;
                field bits<32> Inst;
                let Inst{31 - 24} = !cond(isY: 0x90, true: 0x72);
                let Inst{23 - 20} = 0x4;
                let Inst{19 - 16} = baseOpCode;
                let Inst{15 - 0} = addr;
            }
        }

        def ispd8 : STM8Inst<mnemonic # "\t$addr",
            (ins DirectOff8SP:$addr), (outs),
            !cond(
                OpNodeInfo<opNode>.IsUndef: [],
                OpNodeInfo<opNode>.HasIn:   [(store (opNode (i8 (load SPDispCP:$addr))), SPDispCP:$addr)],
                true:                       [(store (opNode), SPDispCP:$addr)]
            )> {
            bits<8> addr;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 12} = 0x0;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = addr;
        }

        def im8 : STM8Inst<mnemonic # "\t$addr",
            (ins IndirectMem8:$addr), (outs), []> {
            bits<8> addr;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 12} = 0x923;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = addr;
        }

        def im16 : STM8Inst<mnemonic # "\t$addr",
            (ins IndirectMem16:$addr), (outs), []> {
            bits<16> addr;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 20} = 0x723;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = addr;
        }

    }
}

multiclass GenUnary_Fmt<string mnemonic, SDPatternOperator opNode> {
    let mayLoad = OpNodeInfo<opNode>.HasIn, mayStore = true in {

        def ir : STM8GenInst<mnemonic # "\t$addr",
            (ins IR16:$addr), (outs),
            !cond(
                OpNodeInfo<opNode>.IsUndef: [],
                OpNodeInfo<opNode>.HasIn:   [(store (opNode (i8 (load IR16Pat:$addr))), IR16Pat:$addr)],
                true:                       [(store (opNode), IR16Pat:$addr)]
            )>;

        def ird8 : STM8GenInst<mnemonic # "\t$addr",
            (ins IR16D8:$addr), (outs),
            !cond(
                OpNodeInfo<opNode>.IsUndef: [],
                OpNodeInfo<opNode>.HasIn:   [(store (opNode (i8 (load XYDispCP:$addr))), XYDispCP:$addr)],
                true:                       [(store (opNode), XYDispCP:$addr)]
            )>;

        def ird16 : STM8GenInst<mnemonic # "\t$addr",
            (ins IR16D16:$addr), (outs),
            !cond(
                OpNodeInfo<opNode>.IsUndef: [],
                OpNodeInfo<opNode>.HasIn:   [(store (opNode (i8 (load XYDispCP:$addr))), XYDispCP:$addr)],
                true:                       [(store (opNode), XYDispCP:$addr)]
            )>;
    }
}

let Defs = [CCv, CCn, CCz] in {
    defm INC    : Unary_Fmt<"inc ", 0xC,  IncPatFrag>;
    defm GenINC : GenUnary_Fmt<"inc", IncPatFrag>;

    defm DEC    : Unary_Fmt<"dec ", 0xA,  DecPatFrag>;
    defm GenDEC : GenUnary_Fmt<"dec", DecPatFrag>;
}

let Defs = [CCn, CCz], isReMaterializable = true in {

    defm CLR    : Unary_Fmt<"clr ", 0xF,  ClrPatFrag8>;
    defm GenCLR : GenUnary_Fmt<"clr", ClrPatFrag8>;
}

let Defs = [CCv, CCn, CCz, CCc] in {
    defm NEG    : Unary_Fmt<"neg ", 0x0,  NegPatFrag>;
    defm GenNEG : GenUnary_Fmt<"neg", NegPatFrag>;
}

let Defs = [CCn, CCc] in {
    defm CPL    : Unary_Fmt<"cpl ", 0x3,  CPLPatFrag8>;
    defm GenCPL : GenUnary_Fmt<"cpl", CPLPatFrag8>;
}

// Shift instructions

let Defs = [CCn, CCz, CCc] in {
    defm SLL    : Unary_Fmt<"sll ", 0x8, STM8SHL>;
    defm GenSLL : GenUnary_Fmt<"sll ", STM8SHL>;

    defm SRL    : Unary_Fmt<"srl ", 0x4, STM8SHR>;
    defm GenSRL : GenUnary_Fmt<"srl ", STM8SHR>;

    defm SRA    : Unary_Fmt<"sra ", 0x7, STM8ASHR>;
    defm GenSRA : GenUnary_Fmt<"sra ", STM8ASHR>;
}

def : MnemonicAlias<"sla", "sll">;

// Rotate through Carry instructions
let Uses = [CCc], Defs = [CCn, CCz, CCc] in {
    defm RLC    : Unary_Fmt<"rlc ", 0x9, undef>;
    defm GenRLC : GenUnary_Fmt<"rlc ", undef>;

    defm RRC    : Unary_Fmt<"rrc ", 0x6, undef>;
    defm GenRRC : GenUnary_Fmt<"rrc ", undef>;
}

////////////////////////////////////////////////////////////////////////////////
// 16-bit increment/decrement
////////////////////////////////////////////////////////////////////////////////

multiclass UnaryW_Fmt<string mnemonic, bits<4> baseOpCode, SDPatternOperator opNode> {

    foreach RC = [XREG, YREG] in {

        def RC.RS : STM8Inst<mnemonic # "\t$dst",
            !cond(
                OpNodeInfo<opNode>.HasIn: (ins RC:$src),
                true:                     (ins),
            ),
            (outs RC:$dst), []> {
            
            let Constraints = OpNodeInfo<opNode>.DstSrcConstraint;

            let mayLoad = OpNodeInfo<opNode>.HasIn;
            let mayStore = true;

            let Size = !add(1, !eq(RC, YREG));
            field bits<16> Inst;
            let Inst{15 - 4} = 0x905;
            let Inst{3 - 0} = baseOpCode;
        }
    }
}

multiclass GenUnaryW_Fmt<string mnemonic, SDPatternOperator opNode> {

    def r : STM8GenInst<mnemonic # "\t$dst",
        !cond(
            OpNodeInfo<opNode>.HasIn: (ins XY:$src),
            true:                     (ins),
        ),
        (outs XY:$dst),
        !cond(
            OpNodeInfo<opNode>.IsUndef: [],
            OpNodeInfo<opNode>.HasIn:   [(set i16:$dst, (opNode i16:$src))],
            true:                       [(set i16:$dst, (opNode))]
        )> {
        
        let Constraints = OpNodeInfo<opNode>.DstSrcConstraint;
    }
}

let Defs = [CCv, CCn, CCz] in {
    defm INCW    : UnaryW_Fmt<"incw", 0xC, IncPatFrag>;
    defm GenINCW : GenUnaryW_Fmt<"incw", IncPatFrag>;

    defm DECW    : UnaryW_Fmt<"decw", 0xA, DecPatFrag>;
    defm GenDECW : GenUnaryW_Fmt<"decw", DecPatFrag>;
}

let Defs = [CCn, CCz], isReMaterializable = true in {
    defm CLRW    : UnaryW_Fmt<"clrw", 0xF, ClrPatFrag16>;
    defm GenCLRW : GenUnaryW_Fmt<"clrw", ClrPatFrag16>;
}

let Defs = [CCv, CCn, CCz, CCc] in {
    defm NEGW    : UnaryW_Fmt<"negw", 0x0, NegPatFrag>;
    defm GenNEGW : GenUnaryW_Fmt<"negw", NegPatFrag>;
}

let Defs = [CCn, CCz, CCc] in {
    defm CPLW    : UnaryW_Fmt<"cplw", 0x3, CPLPatFrag16>;
    defm GenCPLW : GenUnaryW_Fmt<"cplw", CPLPatFrag16>;
}

let Defs = [CCn, CCz, CCc] in {
    defm SLLW    : UnaryW_Fmt<"sllw", 0x8, STM8SHL>;
    defm GenSLLW : GenUnaryW_Fmt<"sllw", STM8SHL>;

    defm SRLW    : UnaryW_Fmt<"srlw", 0x4, STM8SHR>;
    defm GenSRLW : GenUnaryW_Fmt<"srlw", STM8SHR>;

    defm SRAW    : UnaryW_Fmt<"sraw", 0x7, STM8ASHR>;
    defm GenSRAW : GenUnaryW_Fmt<"sraw", STM8ASHR>;
}

def : MnemonicAlias<"slaw", "sllw">;

let Uses = [CCc], Defs = [CCn, CCz, CCc] in {
    defm RLCW    : UnaryW_Fmt<"rlcw", 0x9, undef>;
    defm GenRLCW : GenUnaryW_Fmt<"rlcw", undef>;

    defm RRCW    : UnaryW_Fmt<"rrcw", 0x6, undef>;
    defm GenRRCW : GenUnaryW_Fmt<"rrcw", undef>;
}
