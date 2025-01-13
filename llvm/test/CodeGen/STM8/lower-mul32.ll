; RUN: llc < %s -march=stm8 | FileCheck --match-full-lines %s

define i32 @test(i32 %a, i32 %b) {
    %res = mul i32 %a, %b
    ret i32 %res
}

; CHECK: call __mulsi3
; CHECK: ret
