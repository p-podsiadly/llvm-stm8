; RUN: llc < %s -march=stm8 | FileCheck %s

define signext i8 @cmp_eq_imm8(i8 %v) {
entry:
  %cmp = icmp eq i8 %v, 54
  %res = zext i1 %cmp to i8
  ret i8 %res
}

; CHECK-LABEL: cmp_eq_imm8
; CHECK: cp a,#0x36
; CHECK: jreq LBB0_2
; CHECK: clr a
; CHECK: ld a,#0x1
; CHECK: LBB0_3
; CHECK: ret

define signext i16 @cmp_eq_imm8_ret_i16(i8 %v) {
entry:
  %cmp = icmp eq i8 %v, 54
  %res = zext i1 %cmp to i16
  ret i16 %res
}

; CHECK-LABEL: cmp_eq_imm8_ret_i16
; CHECK: cp a,#0x36
; CHECK: jreq LBB1_2
; CHECK: clrw x
; CHECK: ldw x,#0x1
; CHECK: ret

define signext i8 @cmp_ne_imm8(i8 %v) {
entry:
  %cmp = icmp ne i8 %v, 54
  %res = zext i1 %cmp to i8
  ret i8 %res
}

; CHECK-LABEL: cmp_ne_imm8
; CHECK: cp a,#0x36
; CHECK: jrne

define signext i8 @cmp_slt_imm8(i8 %v) {
entry:
  %cmp = icmp slt i8 %v, 54
  %res = zext i1 %cmp to i8
  ret i8 %res
}

; CHECK-LABEL: cmp_slt_imm8
; CHECK: cp a,#0x36
; CHECK: jrslt

define signext i8 @cmp_ult_imm8(i8 %v) {
entry:
  %cmp = icmp ult i8 %v, 54
  %res = zext i1 %cmp to i8
  ret i8 %res
}

; CHECK-LABEL: cmp_ult_imm8
; CHECK: cp a,#0x36
; CHECK: jrc

define signext i8 @cmp_sgt_imm8(i8 %v) {
entry:
  %cmp = icmp sgt i8 %v, 54
  %res = zext i1 %cmp to i8
  ret i8 %res
}

; CHECK-LABEL: cmp_sgt_imm8
; CHECK: cp a,#0x36
; CHECK: jrsgt

define signext i8 @cmp_ugt_imm8(i8 %v) {
entry:
  %cmp = icmp ugt i8 %v, 54
  %res = zext i1 %cmp to i8
  ret i8 %res
}

; CHECK-LABEL: cmp_ugt_imm8
; CHECK: cp a,#0x36
; CHECK: jrugt

define signext i8 @cmp_eq_imm16(i16 %v) {
entry:
  %cmp = icmp eq i16 %v, 31732
  %res = zext i1 %cmp to i8
  ret i8 %res
}

; CHECK-LABEL: cmp_eq_imm16
; CHECK: cpw x,#0x7bf4
; CHECK: jreq
