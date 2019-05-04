#!/bin/sh

if [ $# -lt 1 ] 
then
    echo "Run MacSim tests"
    echo "Usage: $0 [options...] <architecture> <test [...]>"
    echo "Options are forwarded to MacSim"
    echo "Supported architectures: armv3, armv6m, riscv"
    echo
    echo "Tests:"
    echo "  riscv-tests  original tests for RISC-V ISA"
    echo "  seq          simple sequential programs"
    echo "  mpi          message passing interface with 4 cores"
    echo "  fgmp         fine grained message passing interface with 8x8 nodes"
    echo "  noc          very short special tests for a 4x4 NoC"
    echo "  benchmarks   large benchmarks from sw/benchmarks/"
    echo "  copper       large benchmarks with fixed problem size for FPGA"

    exit
fi



error () {
    TESTNO=$((${TESTNO}+1))
    echo not ok ${TESTNO} - "\033[0;31m" $@ "\033[0m"
#    cat simout.tmp
    echo "1..${TESTNO} # One test failed, stopping."
    exit
}


passed () {
    TESTNO=$((${TESTNO}+1))
    echo "ok ${TESTNO} - Functional correctness test $1 \033[0;32mpassed.\033[0m"
}


check_core0 () {
    grep "iter " coreout.0.tmp | diff benchmarks/$1.correct - \
      || error "$1: wrong output on core 0 "
    passed $1
}


check_verification () {
    if grep -q "verification successful"  coreout.0.tmp
    then
        passed $1
    else
        error "$1: verification failed"
    fi
}


RCMC_ROOT=`dirname $0`/../..
SIMULATOR="${RCMC_ROOT}/macsim/build/macsim-seq"


# options for MacSim
while [ $# -gt 0 ]
do
    case "$1" in
        -*) # options for MacSim
            OPTIONS="$OPTIONS $1"
            shift
            ;;
        *)  break
            ;;
    esac
done


# architecture
ARCH="$1"
case "$ARCH" in
    armv3) ;;
    armv6m) ;;
    riscv) ;;
    rv64i) ;;
#    rvmpb) ;;
    *)
        echo "Unknown architecture '${ARCH}'"
        exit
        ;;
esac
export ARCH="$ARCH"
shift


# build libraries
make --silent -C ${RCMC_ROOT}/sw/lib/arch/${ARCH}


# tests
TESTNO=0
while [ $# -gt 0 ]
do
    case "$1" in
        -*) # options for MacSim
            OPTIONS="$OPTIONS $1"
            ;;

        riscv-tests) # test from the RISC-V repository
            make --silent -C ${RCMC_ROOT}/sw/tests/riscv-tests/ clean all
            for ELF in ${RCMC_ROOT}/sw/tests/riscv-tests/build/*.${ARCH}.elf
            do
                NAME=`basename ${ELF} .${ARCH}.elf`
                ${SIMULATOR} -A${ARCH} -N1 -a${ELF} \
                    -ocoreout.tmp ${OPTIONS} -g -q > simout.tmp
                echo "k" | diff - coreout.tmp || error "${NAME}: wrong output"
                passed ${NAME}
            done
            ;;

        seq) # miscellaneous sequential single core test
            make --silent -C ${RCMC_ROOT}/sw/tests/seq/ clean all
            for ELF in ${RCMC_ROOT}/sw/tests/seq/build/*.${ARCH}.elf
            do
                NAME=`basename ${ELF} .${ARCH}.elf`
                ${SIMULATOR} -A${ARCH} -N1 -a${ELF} -P "arg1 2 arg3" \
                    -ocoreout.tmp ${OPTIONS} -g -q > simout.tmp
                diff seq/${NAME}.0.check coreout.tmp || error "${NAME}: wrong output"
                passed ${NAME}
            done
            ;;

        mpi) # message passing interface
             # running with 2x2 cores and checking all outpute
            make --silent -C ${RCMC_ROOT}/sw/tests/mpi/ clean all
            for ELF in ${RCMC_ROOT}/sw/tests/mpi/build/*.${ARCH}.elf
            do
                NAME=`basename ${ELF} .${ARCH}.elf`
                ${SIMULATOR} -A${ARCH} -N4 -a${ELF} -c0 -mcoreout.0.tmp \
                    -c1 -mcoreout.1.tmp -c2 -mcoreout.2.tmp -c3 -mcoreout.3.tmp \
                    ${OPTIONS} -g -q > simout.tmp
                for CORE in 0 1 2 3
                do
                    diff mpi/${NAME}.${CORE}.check coreout.${CORE}.tmp \
                      || error "${NAME}: wrong output on core $CORE "
                done
                passed ${NAME}
            done
            ;;

        mpi-fc) # message passing interface with one-to-one flow control
             # running with 2x2 cores and checking all outpute
            make ADD_CFLAGS=-DRCMC_MPI_FLOWCONTROL \
                -C ${RCMC_ROOT}/sw/tests/mpi/ -f ../../all_single_c_files.mk clean all
            for ELF in ${RCMC_ROOT}/sw/tests/mpi/build/*.${ARCH}.elf
            do
                NAME=`basename ${ELF} .${ARCH}.elf`
                ${SIMULATOR} -A${ARCH} -N4 -a${ELF} -Rpnoo -c0 -mcoreout.0.tmp \
                    -c1 -mcoreout.1.tmp -c2 -mcoreout.2.tmp -c3 -mcoreout.3.tmp \
                    ${OPTIONS} -g -q > simout.tmp
                for CORE in 0 1 2 3
                do
                    diff mpi/${NAME}.${CORE}.check coreout.${CORE}.tmp \
                      || error "${NAME}: wrong output on core $CORE "
                done
                passed ${NAME}
            done
            ;;

        fgmp) # fine grained message passing
              # running with 8x8 cores and checking output of core 0
            make --silent -C ${RCMC_ROOT}/sw/tests/fgmp/ clean all
            for ELF in ${RCMC_ROOT}/sw/tests/fgmp/build/*.${ARCH}.elf
            do
                NAME=`basename ${ELF} .${ARCH}.elf`
                ${SIMULATOR} -A${ARCH} -N64 -a${ELF} \
                    -c0 -mcoreout.tmp ${OPTIONS} -g -q > simout.tmp
                echo "k" | diff - coreout.tmp || error "${NAME}: wrong output"
                passed ${NAME}
            done
            ;;

        noc) # very short special tests for a 4x4 NoC
            make --silent -C ${RCMC_ROOT}/sw/tests/noc/ clean all
            for ELF in ${RCMC_ROOT}/sw/tests/noc/build/4x4_*.${ARCH}.elf
            do
                NAME=`basename ${ELF} .${ARCH}.elf`
                ${SIMULATOR} -A${ARCH} -N4x4 -a${ELF} -RPBBGG00 \
                    -c0 -m0.tmp -c1 -m1.tmp -c2 -m2.tmp -c3 -m3.tmp \
                    -c4 -m4.tmp -c5 -m5.tmp -c6 -m6.tmp -c7 -m7.tmp \
                    -c8 -m8.tmp -c9 -m9.tmp -ca -ma.tmp -cb -mb.tmp \
                    -cc -mc.tmp -cd -md.tmp -ce -me.tmp -cf -mf.tmp \
                    ${OPTIONS} -g -q > simout.tmp
                for CORE in 0 1 2 3 4 5 6 7 8 9 a b c d e f
                do
                    echo "k" | diff - ${CORE}.tmp || error "${NAME}: wrong output"
                done
                passed ${NAME}
            done
            ;;

        benchmarks) # use Makefile from sw/benchmarks with libelastic.a
            make --silent -C ${RCMC_ROOT}/sw/benchmarks/ clean fgmp.${ARCH}

            ${SIMULATOR} -A${ARCH} -N16 -a${RCMC_ROOT}/sw/benchmarks/build/ocean.fgmp.${ARCH} \
                -P "ocean 16 5" -c0 -mcoreout.0.tmp ${OPTIONS} -g -q > simout.tmp
            grep "iter " coreout.0.tmp \
                | diff ${RCMC_ROOT}/sw/benchmarks/correctness/ocean.correct.5 - \
                || error "ocean: wrong output on core 0 "
            passed ocean

            ${SIMULATOR} -A${ARCH} -N256 -a${RCMC_ROOT}/sw/benchmarks/build/bitonic_sort.fgmp.${ARCH} \
                -P "bitonic_sort 64 0" -c0 -mcoreout.0.tmp ${OPTIONS} -g -q > simout.tmp
            check_verification bitonic_sort

            ${SIMULATOR} -A${ARCH} -N16 -a${RCMC_ROOT}/sw/benchmarks/build/cg.fgmp.${ARCH} \
                -P "cg 16 0" -c0 -mcoreout.0.tmp ${OPTIONS} -g -q > simout.tmp
            check_verification cg

            ;;

        copper) # use own Makefile and libcopper.a to build benchmarks with fixed problem size
            make ARCH=${ARCH} --silent -C benchmarks/

            ${SIMULATOR} -A${ARCH} -N16 -abenchmarks/ocean.${ARCH}.elf \
                -c0 -mcoreout.0.tmp ${OPTIONS} -g -q > simout.tmp
            grep "iter " coreout.0.tmp \
                | diff ${RCMC_ROOT}/sw/benchmarks/correctness/ocean.correct.5 - \
                || error "ocean: wrong output on core 0 "
            passed ocean

            ${SIMULATOR} -A${ARCH} -N256 -abenchmarks/bitonic_sort.${ARCH}.elf \
                -c0 -mcoreout.0.tmp ${OPTIONS} -g -q > simout.tmp
            check_verification bitonic_sort

            ${SIMULATOR} -A${ARCH} -N16 -abenchmarks/cg.${ARCH}.elf \
                -c0 -mcoreout.0.tmp ${OPTIONS} -g -q > simout.tmp
            check_verification cg

            ;;


        rcce) # RCCE message passing interface with 48 cores
#            make ARCH=${ARCH} --silent -C ${RCMC_ROOT}/sw/tests/rcce/ clean all
            make ARCH=${ARCH} --silent -C ${RCMC_ROOT}/sw/tests/rcce/

            for ELF in ${RCMC_ROOT}/sw/tests/rcce/build/*.${ARCH}.elf
            do
                NAME=`basename ${ELF} .${ARCH}.elf`
                ${SIMULATOR} -A${ARCH} -N48 -a${ELF} \
                    -c0 -mcoreout.tmp ${OPTIONS} -g -q > simout.tmp
                diff rcce/${NAME}.0.check coreout.tmp \
                    || error "${NAME}: wrong output on core 0 "
                passed ${NAME}
            done
            ;;

        *)  echo "Unknown test '$1'"
            exit
            ;;
    esac
    shift
done
rm -f *.tmp

echo "1..${TESTNO} # Successful!"


