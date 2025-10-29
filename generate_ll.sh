#!/bin/bash

# Path to LLVM build folder
LLVM_BUILD="/home/nemanja/Desktop/Konstrukcija Kompilatora/LLVM/llvm-project/build"

# Check for argument
if [ $# -lt 1 ]; then
    echo "Usage: ./generate_ll.sh <cpp_file>"
    echo "Example: ./generate_ll.sh tests/input.cpp"
    exit 1
fi

CPP_FILE=$1
BASENAME=$(basename "$CPP_FILE" .cpp)
LL_FILE="tests/${BASENAME}.ll"

# Remove any old .ll files
rm -f tests/*.ll

# Compile .cpp → .ll
"$LLVM_BUILD/bin/clang" -emit-llvm -O0 -S "$CPP_FILE" -o "$LL_FILE"

echo "[+] Generated $LL_FILE"
