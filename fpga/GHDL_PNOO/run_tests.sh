#!/bin/bash

# Color for messages.
COL_MSG='\033[1;30m'
COL_SUCCESS='\033[0;32m'
COL_FAIL='\033[0;31m'
COL_NONE='\033[0m'
MACSIM="$(pwd)/../../macsim/build/macsim-seq"
TOOLS="$(pwd)/../../tools"

function message {
    echo -ne "${COL_MSG}$1${COL_NONE}"
}

function success {
    echo -ne "${COL_SUCCESS}$1${COL_NONE}"
}

function error {
    echo -ne "${COL_FAIL}$1${COL_NONE}"
}

function draw_progress_bar {
    barWidth=$1
    
    if [ "${barWidth:0:1}" == "." ]
    then
       barWidth="0$barWidth"
    fi
    
    output=""
    for((i=0;i<${barWidth%.*};i++))
    do
        output="$output="
    done
    
    for((i=${barWidth%.*};i<50;i++))
    do
        output="$output "
    done
    
    percent=$(( ${barWidth%.*} * 2 ))
    echo -en "[$output] $percent%\r"
}

function avg_ghdl_size {
    count=0;
    sum=0;    
    for t in $(ps -o size -p $(pidof ghdl) | egrep -o "[[:digit:]]*")
    do
        ((sum+=t))
        ((count++))
    done
     
    echo $(($sum/count))
}

function free_memory {
    cat /proc/meminfo | grep "MemFree" | egrep -o "[[:digit:]]*"
}
 
function limit_cpu {
    while true
    do
        current=$(pidof ghdl | wc -w)
        max=$(nproc)
        if (( current == 0 ))
        then
            break;
        fi
        
        freeMem=$(free_memory)
        avgGHDL=$(avg_ghdl_size)
        if (( freeMem > avgGHDL))
        then
            break;
        fi
        
        sleep 10
    done    
}

for iterationDimension in {6..6}
do        
    for test_file in $(ls tests/src/tests/$1*)
    do      
        test=$(basename $test_file .c)        
        
#         message "Build for $iterationDimension"x"$iterationDimension\n"
        coreDim=$(grep "constant Dimension" src/constants.vhd | grep -o "[[:digit:]]")
        sed -i "s/Dimension : natural := $coreDim/Dimension : natural := $iterationDimension/" src/constants.vhd
        coreDim=$iterationDimension  
        test_it=${test}_$iterationDimension"x"$iterationDimension         
        
        message "~~~~~~~~~~~~~~~~~~"
        success "$test_it"
        message "~~~~~~~~~~~~~~~~~~\n"        
        
        cd build
        make $test
        cd ..     
        
        limit_cpu
        
        date            
        message "Start macsim\n"
        ${MACSIM} -A riscv -N${coreDim}x${coreDim} -Rpnoo -a $(pwd)/tests/bin/${test}.elf -y logs/${test_it}.macsim.regdump -q 1> logs/${test_it}.macsim.log
        MACSIM_CYCLES=$(grep "Max. Execution Time" logs/${test_it}.macsim.log | awk '{ print $4 }')
        message "Simulation took: $MACSIM_CYCLES cycles\n"
        message "Start GHDL\n"
        
         ghdl -r --workdir=build/ghdl top_level --stop-time=$(($MACSIM_CYCLES+4))ns --vcd=logs/${test_it}.vcd  2> logs/${test_it}.ghdl.log &
         sleep 5
        
        
#         |           
#         grep "Now is [[:digit:]]*ns +0"| grep -o "[[:digit:]]*ns" | grep -o "[[:digit:]]*" |             
#         while read line
#         do
#             current=$line
#             max=$(($MACSIM_CYCLES+4))
#             
#             if [ "$current" != "" ]
#             then
#                 draw_progress_bar $(bc -l <<< "$current/$max*50")
#             fi                
#         done
#         message "\nSimulation complete\n"
    done
    
    wait
done

message "Finish executing\n"
    for test_elf in $(ls tests/bin/$1*)
    do       
        errorMsg=""   
        for iterationDimension in {6..6}
        do
            test=$(basename $test_elf .elf)
            test_it=${test}_$iterationDimension"x"$iterationDimension
            
            #$TOOLS/accuracy/vcd2regdump.awk logs/$test_vcd > logs/$test_it.register.log
            $TOOLS/vcd_parser/bin/vcd_parser --format=regdump --scope=reg1.reg1 --signal=reg logs/$test_it.vcd > logs/$test_it.register.log
                
            declare -A coreReg
            while read line
            do        
                if [ "$(echo $line | cut -d" " -f1)" != "" ] 
                then
                    coreReg[$(echo $line | cut -d" " -f1)]=$(echo $line | cut -d" " -f3)        
                else
                    errorMsg="${errorMsg}[$iterationDimension"x"$iterationDimension] Simulation seems to be crashed.\n"
                fi
            done <<< $(grep " x3 " logs/$test_it.register.log)    
            
             
            for i in "${!coreReg[@]}"
            do
                if [ "${coreReg[$i]}" == "ffff000" ]            
                then
                    errorMsg="${errorMsg}[$iterationDimension"x"$iterationDimension] Core $i did not terminate.\n" 
                elif [ "${coreReg[$i]}" != "0" ]            
                then
                    errorMsg="${errorMsg}[$iterationDimension"x"$iterationDimension] Core $i broke assertion at ${coreReg[$i]}\n"
                fi      
            done
            
            if ! cmp --silent logs/$test_it.register.log logs/$test_it.macsim.regdump
            then
                errorMsg="${errorMsg}[$iterationDimension"x"$iterationDimension] Register dumps differ.\n"
            fi
        done
            
        if [ "$errorMsg" == "" ]
        then
            success "Test $test: Ok\n"
        else
            error "Test $test: Fail\n"
            error "$errorMsg\n"
        fi
    done

