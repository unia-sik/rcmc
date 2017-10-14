#!/bin/sh



failed () {
    TESTNO=$((${TESTNO}+1))
    echo not ok ${TESTNO} - Test $@
#    echo "1..${TESTNO} # One test failed, stopping."
#    exit
}



passed () {
    TESTNO=$((${TESTNO}+1))
    echo "ok ${TESTNO} - Test $1 passed."
}



if [ $# -eq 0 ]
then
    echo "Compare execution time for all given executables"
    echo "Usage: $0 <RISC-V ELF files ...>"
    exit
fi

TESTNO=0
for f in "$@"
do
#    # Ignore tests for specific cids.
#    if [[ $f =~ .*_cid[0-9]+.riscv.elf ]]
#    then
#        continue
#    fi

    NAME=$(basename "$f")

    ./cmp_one.sh "$f" $2 $3 1> /dev/null
    result=$?
    if [ $result -eq 0 ]
    then
        passed $NAME
    elif [ $result -eq 1 ]
    then
        failed $NAME " has diffs!"
    elif [ $result -eq 2 ]
    then
        failed $NAME " failed!"
    fi
 done

echo "1..${TESTNO}"
