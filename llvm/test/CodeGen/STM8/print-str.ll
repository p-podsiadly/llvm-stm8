target datalayout = "E-P0-p:16:8-i8:8-i16:8-i32:8-i64:8-f32:8-f64:8-n8:16-a:8"
target triple = "stm8"

; Function Attrs: nounwind
define dso_local void @print_str(ptr nocapture noundef readonly %str) local_unnamed_addr #0 {
entry:
  %0 = load i8, ptr %str, align 1, !tbaa !3
  %tobool.not3 = icmp eq i8 %0, 0
  br i1 %tobool.not3, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %1 = phi i8 [ %2, %for.body ], [ %0, %entry ]
  %str.addr.04 = phi ptr [ %incdec.ptr, %for.body ], [ %str, %entry ]
  tail call void @putchar(i8 noundef signext %1) #2
  %incdec.ptr = getelementptr inbounds i8, ptr %str.addr.04, i16 1
  %2 = load i8, ptr %incdec.ptr, align 1, !tbaa !3
  %tobool.not = icmp eq i8 %2, 0
  br i1 %tobool.not, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body, %entry
  ret void
}

declare dso_local void @putchar(i8 noundef signext) local_unnamed_addr #1

attributes #0 = { nounwind "frame-pointer"="all" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { "frame-pointer"="all" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #2 = { nobuiltin nounwind "no-builtins" }

!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
