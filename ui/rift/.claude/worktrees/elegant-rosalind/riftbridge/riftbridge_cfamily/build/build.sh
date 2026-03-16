#!/bin/bash

# Build script for rift tomography (C, H, CPP, HPP, CSharp models)
# Usage: ./build.sh [lib | c | cpp | cs | all]

TARGET=$1

case $TARGET in
  lib)
    gcc -c ../src/rift.c ../src/riftbridge.c ../src/riftest.c -o rift.o
    ar rcs ../librift.a rift.o
    echo "Built librift.a"
    ;;
  c)
    gcc ../test/main.c -I../include -L.. -lrift -o ../test/main_c
    echo "Built main_c"
    ;;
  cpp)
    g++ ../test/main.cpp -I../include -L.. -lrift -o ../test/main_cpp
    echo "Built main_cpp"
    ;;
  cs)
    csc ../test/main.cs /reference:System.dll /out:../test/main.exe
    echo "Built main.exe (C#)"
    ;;
  all)
    $0 lib
    $0 c
    $0 cpp
    $0 cs
    ;;
  *)
    echo "Usage: $0 [lib | c | cpp | cs | all]"
    ;;
esac