#!/bin/sh
# Fetch, modify and build riscv-tools with support for FGMP instructions


PREFIX=$(pwd)/../riscv
COMMIT=20b5c8a1 # tested commit as of 2017-08-26


for arg in "$@"
do
    case $arg in
        --prefix=*)
            PREFIX="${arg#*=}"
            shift
            ;;
        --commit=*)
            COMMIT="${arg#*=}"
            shift
            ;;
        *)
            echo "Usage: $0 [options]"
            echo "  --prefix=<path> Install to a different path than rcmc/install/riscv/"
            echo "  --commit=<hex>  Use a different commit than the tested one"
            echo "  --commit=master Use latest commit"
            exit 1
    ;;
esac
done


##########################################
# download source code from git repository
##########################################
rm -rf riscv-tools
git clone https://github.com/riscv/riscv-tools.git
cd riscv-tools
git checkout $COMMIT
git submodule update --init --recursive


########################################
# patch for support of FGMP instructions
########################################
cd riscv-gnu-toolchain/riscv-binutils-gdb
../../../modify_fgmp.sh
cd ../..


##################################
# run original RISC-V build script
##################################
export RISCV=$PREFIX
./build.sh
