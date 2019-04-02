#!/bin/sh


failed () {
    TESTNO=$((${TESTNO}+1))
    echo "not ok ${TESTNO} - Test $@"
#    echo "1..${TESTNO} # One test failed, stopping."
#    exit
}



passed () {
    TESTNO=$((${TESTNO}+1))
    echo "ok ${TESTNO} - Test $@"
}



if [ $# -le 3 ]
then
    echo "Compare MacSim vs. VHDL simulation for all given executables"
    echo "Usage: $0 <core name> <noc width> <workdir> <RISC-V ELF files ...>"
    exit
fi


CORENAME="$1"
WIDTH="$2"
WORKDIR="$3"
shift
shift
shift

TESTNO=0
for f in "$@"
do
    NAME=$(basename "$f")

    ./cmp.sh $CORENAME $WIDTH $WORKDIR "$f" 1> /dev/null
    result=$?
    if [ $result -eq 0 ]
    then
        passed $NAME "\033[0;32mpassed.\033[0m"
    elif [ $result -eq 1 ]
    then
        failed $NAME "\033[0;31mhas diffs!\033[0m"
    elif [ $result -eq 2 ]
    then
        failed $NAME "\033[0;31mfailed!\033[0m"
    fi
done

echo "1..${TESTNO}"
