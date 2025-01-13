; RUN: llc < %s -march=stm8 | FileCheck --match-full-lines %s

@arr_i8 = external dso_local global ptr, align 1

define dso_local signext i8 @get_elem_i8(i16 noundef %idx) {
entry:
  %idx.addr = alloca i16, align 1
  store i16 %idx, ptr %idx.addr, align 1
  %0 = load ptr, ptr @arr_i8, align 1
  %1 = load i16, ptr %idx.addr, align 1
  %arrayidx = getelementptr inbounds i8, ptr %0, i16 %1
  %2 = load i8, ptr %arrayidx, align 1
  ret i8 %2
}

; CHECK: ldw  y,arr_i8
; CHECK: ldw  (#0x1,sp),x
; CHECK: addw y,(#0x1,sp)
; CHECK: ld   a,(y)
; CHECK: ret

define dso_local signext i8 @get_elem_i8_const_idx() {
entry:
  %0 = load ptr, ptr @arr_i8, align 1
  %arrayidx = getelementptr inbounds i8, ptr %0, i16 7
  %1 = load i8, ptr %arrayidx, align 1
  ret i8 %1
}

; CHECK: ldw x,arr_i8
; CHECK: ld  a,(#0x7,x)
; CHECK: ret
