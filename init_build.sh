#!/bin/bash
cmake -DCMAKE_CXX_COMPILER=`which clang++` -S . -B bin -G Ninja
