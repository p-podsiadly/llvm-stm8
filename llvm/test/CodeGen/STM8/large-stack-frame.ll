; RUN: llc < %s -march=stm8 | FileCheck %s

; This test verifies that the backend correctly handles SP-relative loads with
; offsets larger than 255. The test function allocates 260 bytes large array on
; the stack, calls fill_array() and use_array() to prevent optimizations and
; finally returns the sum of arr[0], arr[100] and arr[259].

declare void @fill_array(ptr %arr);
declare i16 @use_array(ptr %arr);

define i16 @test() {
  %arr = alloca [300 x i8], align 1
  call void @fill_array(ptr %arr)
  call i16 @use_array(ptr %arr)
  %v0_ptr = getelementptr inbounds [200 x i8], ptr %arr, i16 0, i16 0
  %v0 = load i16, ptr %v0_ptr
  %v1_ptr = getelementptr inbounds [200 x i8], ptr %arr, i16 0, i16 100
  %v1 = load i16, ptr %v1_ptr
  %v2_ptr = getelementptr inbounds [200 x i8], ptr %arr, i16 0, i16 259
  %v2 = load i16, ptr %v2_ptr
  %v0v1 = add i16 %v0, %v1
  %res = add i16 %v0v1, %v2
  ret i16 %res
}

; CHECK:      sub  sp,#0xff
; CHECK-NEXT: sub  sp,#0x31
; CHECK:      call fill_array
; CHECK:      call use_array
; CHECK:      ldw  x,(#0x5,sp)
; CHECK-NEXT: addw x,(#0x69,sp)
; CHECK-NEXT: addw sp,#0xff
; CHECK-NEXT: addw x,(#0x9,sp)
; CHECK-NEXT: sub  sp,#0xff
; CHECK:      addw sp,#0xff
; CHECK-NEXT: addw sp,#0x31
; CHECK-NEXT: ret
