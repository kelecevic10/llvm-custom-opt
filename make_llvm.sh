#!/bin/bash

# Path to LLVM build folder
LLVM_BUILD="/home/nemanja/Desktop/Konstrukcija Kompilatora/LLVM/llvm-project/build"

"$LLVM_BUILD/bin/opt" -load "$PWD/cmake-build-debug/MyDeadCodeEliminationPass.so" -enable-new-pm=0 -dead-code-elimination-pass tests/input.ll -o tests/output.ll -S