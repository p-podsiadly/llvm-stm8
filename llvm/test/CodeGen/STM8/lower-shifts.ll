; RUN: llc < %s -march=stm8 | FileCheck %s

; This test checks handling of both one-bit and variable-bit shifts.

define i8 @shl_1_i8(i8 %a) {
    %res = shl i8 %a, 1
    ret i8 %res
}

; CHECK: sll a
; CHECK: ret

define i8 @lshr_1_i8(i8 %a) {
    %res = lshr i8 %a, 1
    ret i8 %res
}

; CHECK: srl a
; CHECK: ret

define i8 @ashr_1_i8(i8 %a) {
    %res = ashr i8 %a, 1
    ret i8 %res
}

; CHECK: sra a
; CHECK: ret

define i16 @shl_1_i16(i16 %a) {
    %res = shl i16 %a, 1
    ret i16 %res
}

; CHECK: sllw x
; CHECK: ret

define i16 @lshr_1_i16(i16 %a) {
    %res = lshr i16 %a, 1
    ret i16 %res
}

; CHECK: srlw x
; CHECK: ret

define i16 @ashr_1_i16(i16 %a) {
    %res = ashr i16 %a, 1
    ret i16 %res
}

; CHECK: sraw x
; CHECK: ret

define i8 @shl_3_i8(i8 %a) {
    %res = shl i8 %a, 3
    ret i8 %res
}

; CHECK: sll a
; CHECK: sll a
; CHECK: sll a
; CHECK: ret

define i16 @ashr_4_i6(i16 %a) {
    %res = ashr i16 %a, 4
    ret i16 %res
}

; CHECK: sraw x
; CHECK: sraw x
; CHECK: sraw x
; CHECK: sraw x
; CHECK: ret

; Variable shift left on a 8-bit integer
define i8 @shl_var_i8(i8 %a, i8 %b) {
    %res = shl i8 %a, %b
    ret i8 %res
}

; CHECK:       sub   sp,#0x1
; CHECK:       ld    a,(#0x4,sp)
; CHECK-NEXT:  jreq  LBB8_2
; CHECK-NEXT: LBB8_1:
; CHECK:       sll   (#0x1,sp)
; CHECK:       dec   a
; CHECK=NEXT:  jrne  LBB8_1
; CHECK:      LBB8_2:
; CHECK:       addw  sp,#0x1
; CHECK:       ret

; Variable arithmetic shift right on a 16-bit integer
define i16 @ashr_var_i16(i16 %a, i16 %b) {
    %res = ashr i16 %a, %b
    ret i16 %res
}

; CHECK:       ld   a,(#0x4,sp)
; CHECK-NEXT:  jreq LBB9_2
; CHECK:      LBB9_1:
; CHECK:       sraw x
; CHECK:       dec  a
; CHECK-NEXT:  jrne LBB9_1
; CHECK:      LBB9_2:
; CHECK:       ret
