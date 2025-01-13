; RUN: llc < %s -march=stm8 | FileCheck --match-full-lines %s

define i16 @i8_to_i16_zext(i8 %a) {
    %res = zext i8 %a to i16
    ret i16 %res
}

; CHECK: clrw x
; CHECK: ld xl,a
; CHECK: ret

define i16 @i8_to_i16_sext(i8 %a) {
    %res = sext i8 %a to i16
    ret i16 %res
}

; CHECK:  tnz   a
; CHECK:  jrmi  LBB1_2
; CHECK:  clrw  x
; CHECK:  jra   LBB1_3
; CHECK: LBB1_2:
; CHECK:  ldw   x,#0xffff
; CHECK: LBB1_3:
; CHECK:  ld    xl,a
; CHECK:  ret

define i8 @i16_to_i8_trunc(i16 %a) {
    %res = trunc i16 %a to i8
    ret i8 %res
}

; CHECK: ld a,xl
; CHECK: ret
