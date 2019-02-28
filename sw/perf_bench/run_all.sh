#!/bin/bash

# Color for messages.
COL_MSG='\033[1;30m'
COL_SUCCESS='\033[0;32m'
COL_FAIL='\033[0;31m'
COL_NONE='\033[0m'
MACSIM="$(pwd)/../../macsim/build/macsim-seq"
TOOLS="$(pwd)/../../tools"


function limit_cpu {
    while true
    do
        current=$(pidof macsim-seq | wc -w)
        max=$(nproc)
        if (( current < 8 ))
        then
            break;
        fi
                
        sleep 1
    done    
}

cd build
make
cd ..  

for iterationDimension in 4 8 16 32
do        
    for test_file in $(ls src/benchmark/$1*)
    do      
        test=$(basename $test_file .c)        
        coreDim=$iterationDimension  
        test_it=${test}_$iterationDimension"x"$iterationDimension         
        
        echo "Start $test_it"      
           
        
        limit_cpu
        statArg=$(max=$(( 4 * 4  )); for ((i=0;i<$max;i++)); do printf " -c%x -k instr -k traffic " $(( i )); done)
        ${MACSIM} -A riscv -N${coreDim}x${coreDim} -Rpnoo -a $(pwd)/bin/${test}.elf -g $statArg -q 1> logs/${test_it}.macsim.log &
#         sleep 1
    done
done
    
wait

echo "Finish executing\n"



