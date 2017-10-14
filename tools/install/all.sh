#!/bin/sh
# Build all required tools

./build_ghdl.sh
./build_riscv_tools.sh
./build_riscv_gcc.sh --arch=rv64i --prefix=$(pwd)/../rv64i


