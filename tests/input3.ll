; ModuleID = 'tests/input3.cpp'
source_filename = "tests/input3.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; C(x, y) -> koristi samo x
define dso_local i32 @C(i32 %0, i32 %1) #0 {
entry:
  %2 = add nsw i32 %0, 10
  ret i32 %2
}

; B(a, b, c) -> koristi a i c (ali c se u C ne koristi, dakle mrtav posredno)
define dso_local i32 @B(i32 %0, i32 %1, i32 %2) #0 {
entry:
  %3 = call i32 @C(i32 %0, i32 %2)
  %4 = mul nsw i32 %3, 2
  ret i32 %4
}

; A(p, q, r, s) -> koristi p i q; r, s mrtvi
define dso_local i32 @A(i32 %0, i32 %1, i32 %2, i32 %3) #0 {
entry:
  %4 = call i32 @B(i32 %0, i32 %1, i32 %2)
  ret i32 %4
}

; main poziva A(1,2,3,4)
define dso_local i32 @main() #1 {
entry:
  %0 = call i32 @A(i32 1, i32 2, i32 3, i32 4)
  ret i32 %0
}

attributes #0 = { noinline nounwind optnone uwtable }
attributes #1 = { noinline nounwind optnone uwtable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 10.0.1"}
