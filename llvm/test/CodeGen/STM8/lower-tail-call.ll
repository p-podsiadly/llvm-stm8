; RUN: llc < %s -march=stm8 | FileCheck %s

; TODO: Tail calls are currently unsupported.
;       This test verifies that tail calls
;       do not cause runtime error.

declare i8 @callee(i8 %a, i8 %b);

define i8 @caller(i8 %a, i8 %b) {
    %res = tail call i8 @callee(i8 %a, i8 %b)
    ret i8 %res
}

; CHECK: caller:
; CHECK:  call callee
; CHECK:  ret
