//==============================================================================
// Pattern fragments for integer extension
//==============================================================================

def Trunc16to8 : OutPatFrag<(ops node:$src),
    (GenLDarl (i16 node:$src))>;

def AnyExt8to16 : OutPatFrag<(ops node:$src),
    (GenLDrla (i16 (IMPLICIT_DEF)), (i8 node:$src))>;

def ZeroExt8to16 : OutPatFrag<(ops node:$src),
    (GenLDrla (GenCLRWr), (i8 node:$src))>;

//==============================================================================
// ISel patterns for integer truncation/extension
//==============================================================================

def : Pat<(i8 (trunc i16:$src)), (Trunc16to8 XY:$src)>;
def : Pat<(i16 (anyext i8:$src)), (AnyExt8to16 AREG:$src)>;
def : Pat<(i16 (zext i8:$src)), (ZeroExt8to16 AREG:$src)>;

//==============================================================================
// Load Effective Address pseudo-instruction
//==============================================================================

def LoadEffectiveAddr : STM8Pseudo<"lea\t$dst,$base,$disp",
    (ins XY:$base, Imm8:$disp), (outs XY:$dst)> {

    // Same as defs of ADDW
    let Defs = [CCv, CCh, CCn, CCz, CCc];
}
