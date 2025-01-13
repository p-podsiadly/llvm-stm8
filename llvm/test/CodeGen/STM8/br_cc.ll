; RUN: llc < %s -march=stm8 | FileCheck %s

define i16 @test(i16 noundef %v) {
    %tobool = icmp ne i16 %v, 0
    br i1 %tobool, label %ret5, label %ret0

ret5:
    ret i16 5
ret0:
    ret i16 0
}

; CHECK:       tnzw x
; CHECK-NEXT:  jreq LBB0_2
; CHECK:       ldw  x,#0x5
; CHECK:       ret
; CHECK:      LBB0_2
; CHECK:       clrw x
; CHECK:       ret
