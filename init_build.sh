#!/bin/bash
[[ -d ./bin ]] && rm -rf ./bin
cmake -DCMAKE_C_COMPILER=`which clang` -DCMAKE_CXX_COMPILER=`which clang++` -S . -B bin -G Ninja
#cmake -DCMAKE_C_COMPILER=`which gcc` -DCMAKE_CXX_COMPILER=`which g++` -S . -B bin -G Ninja
