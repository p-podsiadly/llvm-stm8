; RUN: llc < %s -march=stm8 | FileCheck %s

define i8 @add8_reg_reg(i8 %a, i8 %b) {
    %result = add i8 %a, %b
    ret i8 %result
}

; CHECK-LABEL: add8_reg_reg:
; CHECK: add a,(#0x3,sp)

define i16 @add16_reg_reg(i16 %a, i16 %b) {
    %result = add i16 %a, %b
    ret i16 %result
}

; CHECK-LABEL: add16_reg_reg:
; CHECK: addw x,(#0x3,sp)
