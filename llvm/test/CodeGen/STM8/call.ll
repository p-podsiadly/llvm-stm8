; RUN: llc < %s -march=stm8 | FileCheck %s

declare dso_local signext i8 @callee_i8_def_cc(i8 noundef signext)

define dso_local signext i8 @call_i8_def_cc() {
  %res = call signext i8 @callee_i8_def_cc(i8 noundef signext 7)
  ret i8 %res
}

; CHECK-LABEL: call_i8_def_cc
; CHECK:       ld a,#0x7
; CHECK:       call callee_i8_def_cc
; CHECK:       ret

declare dso_local sdcccall0 signext i8 @callee_i8_sdcccall0(i8 noundef signext)

define dso_local signext i8 @call_i8_sdcccall0() {
  %res = call sdcccall0 signext i8 @callee_i8_sdcccall0(i8 noundef signext 7)
  ret i8 %res
}

; CHECK-LABEL: call_i8_sdcccall0
; CHECK:       sub sp,#0x2
; CHECK:       ld (#0x1,sp),a
; CHECK:       call callee_i8_sdcccall0
; CHECK:       addw sp,#0x2
; CHECK:       ret

declare dso_local sdcccall1 signext i8 @callee_i8_sdcccall1(i8 noundef signext)

define dso_local signext i8 @call_i8_sdcccall1() {
  %res = call sdcccall1 signext i8 @callee_i8_sdcccall1(i8 noundef signext 7)
  ret i8 %res
}

; CHECK-LABEL: call_i8_sdcccall1
; CHECK:       ld a,#0x7
; CHECK:       call callee_i8_sdcccall1
; CHECK:       ret

declare dso_local sdcccall1 signext i8 @sp_adjustment_callee(i8 %in)

define dso_local signext i8 @sp_adjustment_test() {
  %res = call sdcccall1 signext i8 @sp_adjustment_callee(i8 7)
  %res1 = add i8 %res, 2
  %res2 = call sdcccall1 signext i8 @sp_adjustment_callee(i8 %res1)
  %res3 = add i8 %res2, 2
  %res4 = call sdcccall1 signext i8 @sp_adjustment_callee(i8 %res3)
  ret i8 %res4
}

; CHECK-LABEL: sp_adjustment_test
; CHECK:       sub sp,#0x4
; CHECK:       ld a,#0x7
; CHECK:       ld (#0x1,sp),a
; CHECK:       call sp_adjustment_callee
; CHECK:       add a,#0x2
; CHECK:       ld (#0x1,sp),a
; CHECK:       call sp_adjustment_callee
; CHECK:       add a,#0x2
; CHECK:       ld (#0x1,sp),a
; CHECK:       call sp_adjustment_callee
; CHECK:       addw sp,#0x4
; CHECK:       ret
