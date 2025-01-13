; RUN: llc < %s -march=stm8 | FileCheck %s

define i32 @add_i32(i32 %a, i32 %b) {
    %res = add i32 %a, %b
    ret i32 %res
}

; CHECK-LABEL: add_i32:
; CHECK:       sub   sp,#0x8
; CHECK:       addw  x,(#0xf,sp)
; CHECK:       cpw   x,(#0x7,sp)
; CHECK-NEXT:  jrc   LBB0_2
; CHECK:       clrw  y
; CHECK-NEXT:  jra   LBB0_3
; CHECK-LABEL: LBB0_2:
; CHECK-NEXT:  ldw   y,#0x1
; CHECK-LABEL: LBB0_3:
; CHECK:       addw  x,(#0xd,sp)
; CHECK:       ldw   (#0x5,sp),y
; CHECK-NEXT:  addw  x,(#0x5,sp)
; CHECK:       ret
