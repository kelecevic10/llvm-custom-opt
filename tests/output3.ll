; ModuleID = '../tests/input3.ll'
source_filename = "tests/input3.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
declare dso_local i32 @C.old(i32, i32) #0

; Function Attrs: noinline nounwind optnone uwtable
declare dso_local i32 @B.old(i32, i32, i32) #0

; Function Attrs: noinline nounwind optnone uwtable
declare dso_local i32 @A.old(i32, i32, i32, i32) #0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %0 = call i32 @A(i32 1)
  ret i32 %0
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @C(i32 %0) #0 {
entry:
  %1 = add nsw i32 %0, 10
  ret i32 %1
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @B(i32 %0) #0 {
entry:
  %1 = call i32 @C(i32 %0)
  %2 = mul nsw i32 %1, 2
  ret i32 %2
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @A(i32 %0) #0 {
entry:
  %1 = call i32 @B(i32 %0)
  ret i32 %1
}

attributes #0 = { noinline nounwind optnone uwtable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 10.0.1"}
