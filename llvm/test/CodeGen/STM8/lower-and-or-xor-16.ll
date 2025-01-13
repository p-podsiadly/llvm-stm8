; RUN: llc < %s -march=stm8 | FileCheck %s

; Checks if 16-bit AND/OR/XOR is lowered to two 8-bit ORs LD instruction
; to copy between A reg and low/high parts of X and Y registers.
; Note: 0x0 operand corresponds to B0 imaginary register.

define i16 @test_and(i16 %a, i16 %b) {
    %res = and i16 %a, %b
    ret i16 %res
}

; CHECK: ld a,yh
; CHECK: ld a,xh
; CHECK: and a,(#0x7,sp)

; CHECK: ld a,yl
; CHECK: ld a,xl
; CHECK: and a,(#0x6,sp)

; CHECK: ld xl,a
; CHECK: ld xh,a

define i16 @test_or(i16 %a, i16 %b) {
    %res = or i16 %a, %b
    ret i16 %res
}

; CHECK: ld a,yh
; CHECK: ld a,xh
; CHECK: or a,(#0x7,sp)

; CHECK: ld a,yl
; CHECK: ld a,xl
; CHECK: or a,(#0x6,sp)

; CHECK: ld xl,a
; CHECK: ld xh,a

define i16 @test_xor(i16 %a, i16 %b) {
    %res = xor i16 %a, %b
    ret i16 %res
}

; CHECK: ld a,yh
; CHECK: ld a,xh
; CHECK: xor a,(#0x7,sp)

; CHECK: ld a,yl
; CHECK: ld a,xl
; CHECK: xor a,(#0x6,sp)

; CHECK: ld xl,a
; CHECK: ld xh,a
