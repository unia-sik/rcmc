#!/bin/sh
# Build executables for the cycle-by-cycle comparison of MacSim and the FPGA.
# They are special, because the FGPA has many restrictions (RV64I, slow, ...)

if [ ! -d "./build" ]; then
	mkdir build
fi

BUILD_DIR="build"
RISCVTESTS_DIR="../../baremetal/tests/riscv-tests"
DIR_4X4="../../baremetal/tests/4x4"
MPI_DIR="../../baremetal/tests/mpi"


# from arch_dependent.mk
CC="../rv64i/bin/riscv64-unknown-elf-gcc"
CFLAGS="-march=rv64i"


# Original RISC-V tests for integer ISA subset RV64I
for f in $RISCVTESTS_DIR/rv64ui/*.S
do
    NAME=$(basename "$f" .S)
#    echo $CC $CFLAGS -nostdlib -T$RISCVTESTS_DIR/link.ld \
#        -o $BUILD_DIR/rv64ui_$NAME.rv64i.elf $f \
#        -I$RISCVTESTS_DIR -I$RISCVTESTS_DIR/macros/scalar
    echo $NAME
    $CC $CFLAGS -nostdlib -T$RISCVTESTS_DIR/link.ld \
        -o $BUILD_DIR/rv64ui_$NAME.rv64i.elf $f \
        -I$RISCVTESTS_DIR -I$RISCVTESTS_DIR/macros/scalar
done


## Original RISC-V tests for integer ISA subset RV64M
#for f in $RISCVTESTS_DIR/rv64um/*.S
#do
#    NAME=$(basename "$f" .S)
#    $CC $CFLAGS -nostdlib -T$RISCVTESTS_DIR/link.ld \
#        -o $BUILD_DIR/rv64um_$NAME.rv64im.elf $f \
#        -I$RISCVTESTS_DIR -I$RISCVTESTS_DIR/macros/scalar
#done


CFLAGS="$CFLAGS \
    -T../../baremetal/arch/rv64i/minimal.ld \
    -I../../baremetal/arch/rv64i/include \
    -I../../baremetal/arch/independent/include \
    -Wall -O2 -g"
LDFLAGS="-L../../baremetal/arch/rv64i/lib -nostdlib \
    -lsilver -lc -lm -lgcc -lcopper"


# Special tests for 4x4 NoC
for f in $DIR_4X4/*.c
do
    NAME=$(basename "$f" .c)
    echo $NAME
    $CC $CFLAGS -o $BUILD_DIR/4x4_$NAME.rv64i.elf $f $LDFLAGS
done


# MPI
for f in $MPI_DIR/*.c
do
    NAME=$(basename "$f" .c)
    $CC $CFLAGS -DQUIET -DSAT_SIZE=5 -o $BUILD_DIR/$NAME.rv64i.elf $f $LDFLAGS
        # SAT_SIZE only for mpi_gatherv
done


# Plansch
$CC $CFLAGS -DDEFAULT_LOG2_P=4 -DDEFAULT_LOG2_N=3 \
    -o $BUILD_DIR/ocean_p16n3.rv64i.elf $DIR_4X4/ocean.c $LDFLAGS
