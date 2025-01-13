class STM8Reg<string name, list<Register> subRegs = []>
    : RegisterWithSubRegs<name, subRegs> {
  let Namespace = "STM8";
  let SubRegs = subRegs;
}

// Sub-register indices.
let Namespace = "STM8" in {
  def sub_lo : SubRegIndex<8, 0>;  // Low 8 bits of a register
  def sub_hi : SubRegIndex<8, 8>;  // High 8 bits of a register
  def sub_ex : SubRegIndex<8, 16>; // Highest 8 bits of 24-bit PC register (PC
                                   // Extended)
}

def A : STM8Reg<"a">;
def B : STM8Reg<"b">;

def XL : STM8Reg<"xl">;
def XH : STM8Reg<"xh">;
def X : STM8Reg<"x", [XL, XH]> { let SubRegIndices = [sub_lo, sub_hi]; }

def YL : STM8Reg<"yl">;
def YH : STM8Reg<"yh">;
def Y : STM8Reg<"y", [YL, YH]> { let SubRegIndices = [sub_lo, sub_hi]; }

def Z : STM8Reg<"z">;

def PCL : STM8Reg<"pcl">;
def PCH : STM8Reg<"pch">;
def PCE : STM8Reg<"pce">;
def PC : STM8Reg<"pc", [PCL, PCH, PCE]> {
  let SubRegIndices = [sub_lo, sub_hi, sub_ex];
}

def SP : STM8Reg<"sp">;

//==============================================================================
// Register Classes
//==============================================================================

class STM8RegClass<string name, string regStr, ValueType type, dag regList>
    : RegisterClass<"STM8", [type], 8, regList> {
  ValueType ValType = type;

  // Descriptive name, used by AsmOperand subclasses.
  field string Name = name;
  // String indicating allowed registers, used in instruction names.
  // For example "a" in "ADDai8" or "r" in "PseudoLDWrr".
  field string RS = regStr;
}

// Register classes corresponding to CPE registers.

def AREG : STM8RegClass<"A", "a", i8, (add A)>;
def BREG : STM8RegClass<"B", "b", i8, (add B)>;
def XREG : STM8RegClass<"X", "x", i16, (add X)>;
def YREG : STM8RegClass<"Y", "y", i16, (add Y)>;
def ZREG : STM8RegClass<"Z", "z", i16, (add Z)>;
def SPREG : STM8RegClass<"SP", "sp", i16, (add SP)>;

def XLREG : STM8RegClass<"XL", "xl", i8, (add XL)>;
def XHREG : STM8RegClass<"XH", "xh", i8, (add XH)>;
def YLREG : STM8RegClass<"YL", "yl", i8, (add YL)>;
def YHREG : STM8RegClass<"YH", "yh", i8, (add YH)>;

// Register class for use with generic instructions.
def AB : STM8RegClass<"AB", "r", i8, (add A, B)>;
def XY : STM8RegClass<"XY", "r", i16, (add X, Y)>;
def XYZ : STM8RegClass<"XYZ", "r", i16, (add X, Y, Z)>;

// Catch-all reg class used for Select pseudo-instructions.
def AnyReg : RegisterClass<"STM8", [i8, i16], 8, (add A, B, X, Y, Z)>;

//==============================================================================
// Condition Code register
//
// The CC register is composed of 7 flags. In order to enable more-fine grained
// optimizations it is modelled as 6 independent registers.
//==============================================================================

def CCv : STM8Reg<"ccv">; // Overflow
def CCi : STM8Reg<"cci">; // Two interrupt masks (I1, I2)
def CCh : STM8Reg<"cch">; // Half carry bit
def CCn : STM8Reg<"ccn">; // Negative
def CCz : STM8Reg<"ccz">; // Zero
def CCc : STM8Reg<"ccc">; // Carry

// Note: these two definitions are used only for "push cc" and "pop cc".
def CC : STM8Reg<"cc">;
def CCREG : STM8RegClass<"CC", "cc", i8, (add CC)>;
