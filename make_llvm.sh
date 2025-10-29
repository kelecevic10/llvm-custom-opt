#!/bin/bash

# Path to LLVM build folder
LLVM_BUILD="/home/nemanja/Desktop/Konstrukcija Kompilatora/LLVM/llvm-project/build"

# Check for argument
if [ $# -lt 1 ]; then
    echo "Usage: ./make_llvm.sh <input.ll>"
    echo "Example: ./make_llvm.sh tests/input.ll"
    exit 1
fi

INPUT_LL=$1
OUTPUT_LL="${INPUT_LL%.ll}_output.ll"

"$LLVM_BUILD/bin/opt" -load "$PWD/cmake-build-debug/MyDeadCodeEliminationPass.so" \
  -enable-new-pm=0 -dead-code-elimination-pass "$INPUT_LL" -S -o "$OUTPUT_LL"

echo "[+] Optimized file generated: $OUTPUT_LL"
