; RUN: llc < %s -march=stm8 | FileCheck --match-full-lines %s

define i8 @div_i8_i8_unsigned(i8 %a, i8 %b) {
    %res = udiv i8 %a, %b
    ret i8 %res
}

; CHECK: clrw x
; CHECK: ld xl,a
; CHECK: ld a,(#0x3,sp)
; CHECK: div x,a
; CHECK: ld a,xl
; CHECK: ret

define i8 @rem_i8_i8_unsigned(i8 %a, i8 %b) {
    %res = urem i8 %a, %b
    ret i8 %res
}

; CHECK: clrw x
; CHECK: ld xl,a
; CHECK: ld a,(#0x3,sp)
; CHECK: div x,a
; CHECK: ret

define i16 @div_i16_i16_unsigned(i16 %a, i16 %b) {
    %res = udiv i16 %a, %b
    ret i16 %res
}

; CHECK: divw x,y
; CHECK: ret

define i16 @rem_i16_i16_unsigned(i16 %a, i16 %b) {
    %res = urem i16 %a, %b
    ret i16 %res
}

; CHECK: divw x,y
; CHECK: ldw  x,y
; CHECK: ret

define i16 @div_i16_i8_unsigned(i16 %a, i8 %b) {
    %bw = zext i8 %b to i16
    %res = udiv i16 %a, %bw
    ret i16 %res
}

; CHECK: div x,a
; CHECK: ret

define i16 @rem_i16_i8_unsigned(i16 %a, i8 %b) {
    %bw = zext i8 %b to i16
    %res = urem i16 %a, %bw
    ret i16 %res
}

; CHECK: div  x,a
; CHECK: clrw x
; CHECK: ld   xl,a
; CHECK: ret
