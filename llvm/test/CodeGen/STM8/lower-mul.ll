; RUN: llc < %s -march=stm8 | FileCheck --match-full-lines %s

define i8 @mul_i8_2(i8 %a) {
    %res = mul i8 %a, 2
    ret i8 %res
}

; CHECK: sll a
; CHECK: ret

define i16 @mul_i16_2(i16 %a) {
    %res = mul i16 %a, 2
    ret i16 %res
}

; CHECK: sllw x
; CHECK: ret

define i8 @mul_i8_const_ret_i8(i8 %a) {
    %res = mul i8 %a, 7
    ret i8 %res
}

; CHECK: ld xl,a
; CHECK: ld a,#0x7
; CHECK: mul x,a
; CHECK: ld a,xl
; CHECK: ret

define i16 @mul_i16_i8_ret_i16(i16 %a, i8 %b) {
    %bw = zext i8 %b to i16
    %res = mul i16 %a, %bw
    ret i16 %res
}

; CHECK: call __mulhi3
; CHECK: ret

