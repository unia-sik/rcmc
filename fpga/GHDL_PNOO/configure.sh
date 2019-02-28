#!/bin/bash

rm -r build
mkdir -p build
mkdir -p simulation
mkdir -p logs

cd build
mkdir -p ghdl

if [ "$#" -eq 0 ]
then                             
    cmake -DCMAKE_TOOLCHAIN_FILE=../../sw/arch/rv64i_toolchain.cmake ..
    make
else
    cmake -DCMAKE_TOOLCHAIN_FILE=../../sw/arch/rv64i_toolchain.cmake -DCUSTOM_ELF_FILE=$1 ..
    make custom_elf
fi



