; RUN: llc < %s -march=stm8 | FileCheck %s

define i8 @eliminate_tnz_after_add(i8 %a, i8 %b, i8 %c) optsize {

    %c1 = add i8 %c, 5
    %cmp = icmp sge i8 %c1, 0
    br i1 %cmp, label %ret_a, label %ret_b

ret_a:
    ret i8 %a

ret_b:
    ret i8 %b
}

; Expectations:
; 1. "tnz a " after "add" is eliminated.
; 2. there's a fallthrough to the next basic block (without jra instr)
; CHECK:      add   a,(#0x6,sp)
; CHECK-NEXT: jrmi LBB0_2
; CHECK-NEXT: ; %bb.1:
