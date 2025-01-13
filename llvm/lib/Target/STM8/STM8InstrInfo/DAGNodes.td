/* This file defines Selection DAG nodes specific to the STM8 backend.
 */

def SDT_STM8Wrapper : SDTypeProfile<1, 1, [SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def STM8Wrapper : SDNode<"STM8::WRAPPER", SDT_STM8Wrapper, []>;

def STM8RetGlue : SDNode<"STM8::RET_GLUE", SDTNone,
                         [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def STM8InterruptRetGlue : SDNode<"STM8::IRET_GLUE", SDTNone,
                                  [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Note: must be in sync with STM8::CondCode in STM8ISelLowering.h
def STM8_COND_OTHER : PatLeaf<(i8 0)>;
def STM8_COND_EQ : PatLeaf<(i8 1)>;
def STM8_COND_NE : PatLeaf<(i8 2)>;
def STM8_COND_SLT : PatLeaf<(i8 3)>;
def STM8_COND_SLE : PatLeaf<(i8 4)>;
def STM8_COND_SGT : PatLeaf<(i8 5)>;
def STM8_COND_SGE : PatLeaf<(i8 6)>;
def STM8_COND_ULT : PatLeaf<(i8 7)>;
def STM8_COND_ULE : PatLeaf<(i8 8)>;
def STM8_COND_UGT : PatLeaf<(i8 9)>;
def STM8_COND_UGE : PatLeaf<(i8 10)>;
def STM8_COND_MI : PatLeaf<(i8 11)>;
def STM8_COND_PL : PatLeaf<(i8 12)>;

// Same as STM8_COND_ULT
def STM8_COND_CARRY : PatLeaf<(i8 7)>;

def SDT_STM8Add : SDTypeProfile<2, 2, [
  // Results: sum of operands, CC Carry value (i1)
  SDTCisInt<0>, SDTCisVT<1, i1>,
  // Operands: two integers
  SDTCisSameAs<2, 0>, SDTCisSameAs<3, 0>
]>;

def SDT_STM8AddCarry : SDTypeProfile<2, 3, [
  // Results: sum of operands, CC Carry value (i1)
  SDTCisInt<0>, SDTCisVT<1, i1>,
  // Operands: two integers and incoming CC Carry value (i1)
  SDTCisSameAs<2, 0>, SDTCisSameAs<3, 0>, SDTCisVT<4, i1>
]>;

def STM8Add : SDNode<"STM8::ADD", SDT_STM8Add, []>;
def STM8AddCarry : SDNode<"STM8::ADD_CARRY", SDT_STM8AddCarry, []>;

def SDT_STM8TnzJump : SDTypeProfile<0, 3, [
    // Value to test
    SDTCisInt<0>,
    // CondCode constant
    SDTCisVT<1, i8>,
    // Jump target
    SDTCisVT<2, OtherVT>
]>;

def SDT_STM8CompareJump : SDTypeProfile<0, 4, [
  // LHS & RHS of comparison
  SDTCisInt<0>, SDTCisSameAs<1, 0>,
  // CondCode constant
  SDTCisVT<2, i8>,
  // Jump target
  SDTCisVT<3, OtherVT>
]>;

def STM8TnzJump : SDNode<"STM8::TNZ_JUMP", SDT_STM8TnzJump, [SDNPOutGlue, SDNPHasChain]>;
def STM8CompareJump : SDNode<"STM8::COMPARE_JUMP", SDT_STM8CompareJump, [SDNPOutGlue, SDNPHasChain]>;

def SDT_STM8TnzSelectCC : SDTypeProfile<1, 4, [
  // Result: value of the 3. or 4. operand, depending on the condition
  SDTCisInt<0>,
  // Operand 1: input value to test
  SDTCisInt<1>,
  // Operands 2 and 3: true and false value
  SDTCisSameAs<2, 0>, SDTCisSameAs<3, 0>,
  // Operand 4: Condition code constant
  SDTCisVT<4, i8>
]>;

def SDT_STM8CompareSelectCC : SDTypeProfile<1, 5, [
  // Result: value of the 3. or 4. operand, depending on the condition
  SDTCisInt<0>,
  // Operands 1 and 2: LHS and RHS of comparison
  SDTCisInt<1>, SDTCisSameAs<2, 1>,
  // Operands 3 and 4: true and false value
  SDTCisSameAs<3, 0>, SDTCisSameAs<4, 0>,
  // Operand 5: Condition code constant
  SDTCisVT<5, i8>
]>;

def STM8TnzSelectCC : SDNode<"STM8::TNZ_SELECT_CC", SDT_STM8TnzSelectCC, []>;
def STM8CompareSelectCC : SDNode<"STM8::COMPARE_SELECT_CC", SDT_STM8CompareSelectCC, []>;

//==============================================================================
// Set low/high part of a word
//==============================================================================

def SDT_STM8SetByte : SDTypeProfile<1, 3, [
    // Result: 16-bit integer
    SDTCisVT<0, i16>,
    // Operand 1: 16 bit value
    SDTCisVT<1, i16>,
    // Operand 2: subregister index(0 for low, 1 for high)
    SDTCisInt<2>,
    // Operand 3: 8 bit value (new low/high part)
    SDTCisVT<3, i8>]>;

def SDT_STM8GetByte : SDTypeProfile<1, 2, [
    // Result: 8-bit integer
    SDTCisVT<0, i8>,
    // Operand 1: 16 bit value
    SDTCisVT<1, i16>,
    // Operand 2: subregister index(0 for low, 1 for high)
    SDTCisInt<2>]>;

def STM8SetByte : SDNode<"STM8::SET_BYTE", SDT_STM8SetByte, []>;
def STM8GetByte : SDNode<"STM8::GET_BYTE", SDT_STM8GetByte, []>;

//==============================================================================
// Swap bytes of a 16 bit value
//==============================================================================

def SDT_STM8SwapBytes : SDTypeProfile<1, 1, [SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;

def STM8SwapBytes : SDNode<"STM8::SWAP_BYTES", SDT_STM8SwapBytes, []>;

//==============================================================================
// Shifts
//==============================================================================

// Shift by one bit.
def SDT_STM8Shift : SDTypeProfile<1, 1,
  [SDTCisInt<0>, SDTCisSameAs<1, 0>]>;

// Shift by variable number of bits.
def SDT_STM8VarShift : SDTypeProfile<1, 2,
  [SDTCisInt<0>, SDTCisSameAs<1, 0>, SDTCisVT<2, i8>]>;

def STM8SHL : SDNode<"STM8::SHL", SDT_STM8Shift, []>;
def STM8SHR : SDNode<"STM8::SHR", SDT_STM8Shift, []>;
def STM8ASHR : SDNode<"STM8::ASHR", SDT_STM8Shift, []>;

def STM8VarSHL : SDNode<"STM8::VAR_SHL", SDT_STM8VarShift, [SDNPOutGlue]>;
def STM8VarSHR : SDNode<"STM8::VAR_SHR", SDT_STM8VarShift, [SDNPOutGlue]>;
def STM8VarASHR : SDNode<"STM8::VAR_ASHR", SDT_STM8VarShift, [SDNPOutGlue]>;

def SDT_STM8ShiftParts : SDTypeProfile<2, 3,
  [SDTCisInt<0>, SDTCisSameAs<1, 0>, SDTCisSameAs<2, 0>, SDTCisSameAs<3, 0>, SDTCisInt<4>]>;

def STM8SHL_PARTS : SDNode<"ISD::SHL_PARTS", SDT_STM8ShiftParts, []>;
def STM8SRL_PARTS : SDNode<"ISD::SRL_PARTS", SDT_STM8ShiftParts, []>;

//==============================================================================
// Division with reminder
//==============================================================================

def SDT_STM8UDivRem : SDTypeProfile<2, 2, [
  // Result 1: Quotient
  SDTCisVT<0, i16>,
  // Result 2: Reminder, i8 or i16
  SDTCisInt<1>,
  // Operand 1: Dividend
  SDTCisVT<2, i16>,
  // Operand 2: Divisor, i8 or i16
  SDTCisSameAs<3, 1>]>;

def STM8UDivRem : SDNode<"STM8::UDIVREM", SDT_STM8UDivRem, []>;
