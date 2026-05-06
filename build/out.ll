; ModuleID = 'out.c'
source_filename = "out.c"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30159"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca double, align 8
  store i32 0, ptr %1, align 4
  store i32 5, ptr %2, align 4
  store i32 1, ptr %3, align 4
  store double 1.000000e+00, ptr %4, align 8
  br label %5

5:                                                ; preds = %9, %0
  %6 = load i32, ptr %3, align 4
  %7 = load i32, ptr %2, align 4
  %8 = icmp sle i32 %6, %7
  br i1 %8, label %9, label %16

9:                                                ; preds = %5
  %10 = load double, ptr %4, align 8
  %11 = load i32, ptr %3, align 4
  %12 = sitofp i32 %11 to double
  %13 = fmul double %10, %12
  store double %13, ptr %4, align 8
  %14 = load i32, ptr %3, align 4
  %15 = add nsw i32 %14, 1
  store i32 %15, ptr %3, align 4
  br label %5, !llvm.loop !8

16:                                               ; preds = %5
  %17 = load double, ptr %4, align 8
  %18 = fcmp ogt double %17, 1.000000e+01
  br i1 %18, label %19, label %24

19:                                               ; preds = %16
  %20 = load double, ptr %4, align 8
  %21 = call double @log(double noundef %20) #2
  %22 = call double @exp(double noundef 1.000000e+00) #2
  %23 = fadd double %21, %22
  store double %23, ptr %4, align 8
  br label %27

24:                                               ; preds = %16
  %25 = load double, ptr %4, align 8
  %26 = call double @pow(double noundef %25, double noundef 2.000000e+00) #2
  store double %26, ptr %4, align 8
  br label %27

27:                                               ; preds = %24, %19
  %28 = load double, ptr %4, align 8
  %29 = fptosi double %28 to i32
  ret i32 %29
}

; Function Attrs: nounwind
declare dso_local double @log(double noundef) #1

; Function Attrs: nounwind
declare dso_local double @exp(double noundef) #1

; Function Attrs: nounwind
declare dso_local double @pow(double noundef, double noundef) #1

attributes #0 = { noinline nounwind optnone uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 22.1.5 (https://github.com/llvm/llvm-project 5ea218a153f4d2f815b8244eab3e4b4ba5e00e6c)", isOptimized: false, runtimeVersion: 0, emissionKind: NoDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "out.c", directory: "D:\\CC_Project\\build")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 2}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"uwtable", i32 2}
!6 = !{i32 1, !"MaxTLSAlign", i32 65536}
!7 = !{!"clang version 22.1.5 (https://github.com/llvm/llvm-project 5ea218a153f4d2f815b8244eab3e4b4ba5e00e6c)"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
