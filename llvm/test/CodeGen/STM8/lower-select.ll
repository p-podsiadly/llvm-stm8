; RUN: llc < %s -march=stm8 | FileCheck %s

define i8 @select_i8_eq(i8 %x) {
    %cond = icmp eq i8 %x, 0
    %res = select i1 %cond, i8 u0x38, i8 u0x17
    ret i8 %res
}

; CHECK:       tnz    a
; CHECK-NEXT:  jreq   LBB0_2
; CHECK:       ld     a,#0x17
; CHECK:       jra    LBB0_3
; CHECK:      LBB0_2:
; CHECK:       ld     a,#0x38
; CHECK:      LBB0_3:
; CHECK:       ret

define i16 @select_i16_eq(i16 %x) {
    %cond = icmp eq i16 %x, 0
    %res = select i1 %cond, i16 u0x3800, i16 u0x1700
    ret i16 %res
}

; CHECK:       tnzw x
; CHECK-NEXT:  jreq LBB1_2
; CHECK:       ldw  x,#0x1700
; CHECK:       jra  LBB1_3
; CHECK:      LBB1_2:
; CHECK:       ldw  x,#0x3800
; CHECK:      LBB1_3:
; CHECK:       ret
