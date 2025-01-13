class CCIfIsFirstArg<CCAction Action> : CCIf<"ValNo == 0", Action>;
class CCIfIsSecondArg<CCAction Action> : CCIf<"ValNo == 1", Action>;
class CCIfIsNotFirstOrSecondArg<CCAction Action> : CCIf<"ValNo > 1", Action>;

// Matched if A register has already been allocated
class CCIfAIsAllocated<CCAction Action>
    : CCIf<"State.isAllocated(STM8::A)", Action>;

// Matched if X register has already been allocated
class CCIfXIsAllocated<CCAction Action>
    : CCIf<"State.isAllocated(STM8::X)", Action>;

def CC_SdccCallV0 : CallingConv<[
  // All arguments are passed on stack with default size (0) and alignment (0).
  CCAssignToStack<0, 0>
]>;

// Sub-calling-convention for SDCC v1 non-var-arg functions.
// Defined separately to simplify definition of CC_SdccCallV1.
def CC_SdccCallV1_NonVarArg : CallingConv<[
  // Rules for the 1st argument
  CCIfIsFirstArg<CCIfType<[i8], CCAssignToReg<[A]>>>,
  CCIfIsFirstArg<CCIfType<[i16], CCAssignToReg<[X]>>>,

  // Rules for the 2nd argument:
  // If the 1st arg is i16 and the 2nd srg is i8 => assign to A
  CCIfIsSecondArg<CCIfType<[i8], CCIfXIsAllocated<CCAssignToReg<[A]>>>>,
  // If the 1st arg is i8 and the 2nd arg is i16 => assign to X
  CCIfIsSecondArg<CCIfType<[i16], CCIfAIsAllocated<CCAssignToReg<[X]>>>>,

  // All other cases: use stack
  CCAssignToStack<0, 0>
]>;

def CC_SdccCallV1 : CallingConv<[
  CCIfVarArg<CCAssignToStack<0, 0>>,
  CCIfNotVarArg<CCDelegateTo<CC_SdccCallV1_NonVarArg>>
]>;

//==============================================================================
// Return Value Conventions
//==============================================================================

def RetCC_Sdcc : CallingConv<[
  CCIfType<[i8], CCAssignToReg<[A]>>, CCIfType<[i16], CCAssignToReg<[X]>>,
  CCIfIsSecondArg<CCIfType<[i16], CCAssignToReg<[Y]>>>
]>;

//==============================================================================
// Callee-saved register lists
//==============================================================================

def CSR_Normal : CalleeSavedRegs<(add)> {
  let OtherPreserved = (add B, Z);
}
