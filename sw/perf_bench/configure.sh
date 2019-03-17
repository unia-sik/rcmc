#!/bin/bash

rm -r build
mkdir -p build
cd build

mkdir -p logs

cmake -DCMAKE_TOOLCHAIN_FILE=../../arch/rv64i_toolchain.cmake ..
make



