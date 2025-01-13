; RUN: llc < %s -march=stm8 | FileCheck %s

declare i8 @get_input()
declare i16 @action_1()
declare i16 @action_2()
declare i16 @action_3()
declare i16 @action_4()

define i16 @test() optsize {
    %input = call i8 @get_input()

    switch i8 %input, label %default [
        i8 0, label %case_1
        i8 1, label %case_2
        i8 2, label %case_3
        i8 3, label %case_4
    ]

case_1:
    %res_1 = call i16 @action_1()
    ret i16 %res_1

case_2:
    %res_2 = call i16 @action_2()
    ret i16 %res_2

case_3:
    %res_3 = call i16 @action_3()
    ret i16 %res_3

case_4:
    %res_4 = call i16 @action_4()
    ret i16 %res_4

default:
    ret i16 0
}

; CHECK-LABEL: test:
; CHECK:       call  get_input
; CHECK:       cp    a,#0x3
; CHECK-NEXT:  jrugt LBB0_6
; CHECK:       addw  x,JTI0_0
; CHECK-NEXT:  ldw x,(x)
; CHECK-NEXT:  jp (x)
; CHECK-LABEL: LBB0_2:
; CHECK:       call action_1
; CHECK:       jra LBB0_7
; CHECK-LABEL: LBB0_3:
; CHECK-LABEL: LBB0_4:
; CHECK-LABEL: LBB0_5:
; CHECK-LABEL: LBB0_6:
; CHECK:       clrw  x
; CHECK:       LBB0_7
; CHECK:       ret

; CHECK-LABEL: JTI0_0:
; CHECK-NEXT:  .short LBB0_2
; CHECK-NEXT:  .short LBB0_5
; CHECK-NEXT:  .short LBB0_3
; CHECK-NEXT:  .short LBB0_4
