#!/bin/sh
# Fetch, modify and build only riscv-gcc with support for FGMP instructions


PREFIX=$(pwd)/../rv64imafd
#COMMIT=1a74ccbe # tested commit as of 2017-08-26
#COMMIT=d61f516a # tested commit as of 2017-11-08
COMMIT=afcc8bc
ARCH=rv64imafd


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
        --arch=*)
            ARCH="${arg#*=}"
            shift
            ;;
        *)
            echo "Usage: $0 [options]"
            echo "  --prefix=<path> Install to a different ABSOULTE path than rcmc/install/riscv/"
            echo "  --commit=<hex>  Use a different commit than the tested one"
            echo "  --commit=master Use latest commit"
            echo "  --arch=<isa>    Specify the RISC-V ISA sunset (default: rv64iamfd)"
            exit 1
    ;;
esac
done


##########################################
# download source code from git repository
##########################################
if [ -d risc-gnu-toolchain/.git ]
then
    cd riscv-gnu-toolchain
    git fetch --all
    git reset --hard origin/master
else
    rm -rf riscv-gnu-toolchain
    git clone https://github.com/riscv/riscv-gnu-toolchain
    cd riscv-gnu-toolchain
fi
git checkout ${COMMIT}
git submodule update --init --recursive

########################################
# patch for support of FGMP instructions
########################################
#cd riscv-binutils-gdb
#../../modify_fgmp.sh
#cd ..


############################
# configure and run makefile
############################
mkdir build
cd build
../configure --with-arch="${ARCH}" --prefix="${PREFIX}"
make -j$(nproc)
cd ../..
