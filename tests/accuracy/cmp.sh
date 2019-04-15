#!/bin/sh
# Compare the execution of one benchmark with MacSim and GHDL


# ------------------------------
# additional configuration
# ------------------------------

MEMORY_SIZE=131072
# TODO: ghdl crashes with larger memory

#ROUTER="PUBGG00"
ROUTER=pnoo

CORNERBUF=8
# size of the corner buffer

#GHDL_FLAGS="--ieee=synopsys"
GHDL_FLAGS=""

# ------------------------------






# use pwd to get absolute paths
MYDIR=$(dirname $0)
GHDL="$(pwd)/${MYDIR}/../../tools/ghdl/bin/ghdl"
MACSIM="$(pwd)/${MYDIR}/../../macsim/build/macsim-seq"

# Color for messages.
COL_MSG='\033[1;30m'
COL_SUCCESS='\033[0;32m'
COL_FAIL='\033[0;31m'
COL_NONE='\033[0m'



if [ $# -ne 4 ]
then
    echo "Usage: $0 <core name> <noc width> <work directory> <RISC-V ELF file>"
    echo "Memory size, router configuration and corner buffer size can only be changed in the script"
    exit 1
fi

COREDIR=$(pwd)/${MYDIR}/../../fpga/$1
if [ ! -e "${COREDIR}" ]
then
    echo -e "core name not found!" 1>&2
    exit 2
fi
WIDTH="$2"
WORKDIR="$3"
mkdir -p ${WORKDIR}
ELF="$(pwd)/$4"
if [ ! -e "${ELF}" ]
then
    echo -e "ELF file not found!" 1>&2
    exit 3
fi

cd ${WORKDIR}
rm -f *.cf
TIME_START=$(date +%s)



# MacSim
printf "MacSim simulation"
#${MACSIM} -A riscv -N${WIDTH}x${WIDTH} -J${CORNERBUF} -R${ROUTER} -a ${ELF} -y macsim.regdump -q 1> macsim.log
${MACSIM} -A riscv -N${WIDTH}x${WIDTH} -R${ROUTER} -a ${ELF} -y macsim.regdump -q 1> macsim.log
MACSIM_CYCLES=$(grep "Execution Time" macsim.log | awk '{ print $3 }')
TIME_MACSIM=$(date +%s)
echo "${COL_MSG} ($((TIME_MACSIM-TIME_START)) seconds)${COL_NONE} simulated time: ${COL_MSG}${MACSIM_CYCLES} cycles${COL_NONE}"


# GHDL analysis
printf "GHDL analysis"
${COREDIR}/config.sh ${WIDTH} ${ROUTER} ${CORNERBUF} ${MEMORY_SIZE} ${ELF}
. ./ghdl_analyze.sh # set also GHDL_CYCLE_DRIFT and GHDL_COREID
${GHDL} -e ${GHDL_FLAGS} NoC


# run GHDL simulation
TIME_ANALYSIS=$(date +%s)
echo "${COL_MSG} ($((TIME_ANALYSIS-TIME_MACSIM)) seconds)${COL_NONE}"
printf "GHDL simulation"
${GHDL} -r ${GHDL_FLAGS} NoC --ieee-asserts=disable --stop-time=$(($MACSIM_CYCLES+1+$GHDL_CYCLE_DRIFT))ns --vcd=ghdl.vcd 1> ghdl.log
cd ..


# convert to register dump
TIME_SIM=$(date +%s)
printf "${COL_MSG} ($((TIME_SIM-TIME_ANALYSIS)) seconds = "
echo "$(( MACSIM_CYCLES / (TIME_SIM-TIME_ANALYSIS) )) cycles/second)${COL_NONE}"

printf "Converting VCD to regdump"
#./ghdl_convert.sh ${WORKDIR}/ghdl.vcd > ${WORKDIR}/ghdl.regdump
awk -v drift=${GHDL_CYCLE_DRIFT} -v coreid=${GHDL_COREID} -v width=$2 -f vcd2regdump.awk ${WORKDIR}/ghdl.vcd > ${WORKDIR}/ghdl.regdump
TIME_CONV=$(date +%s)
echo "${COL_MSG} ($((TIME_CONV-TIME_SIM)) seconds)${COL_NONE}"


# comparison
if cmp --silent ${WORKDIR}/macsim.regdump ${WORKDIR}/ghdl.regdump
then
  echo "${COL_SUCCESS}Register dumps are equal!${COL_NONE}"
  exit 0
else
  echo "${COL_FAIL}Register dumps DIFFER!${COL_NONE}"
  exit 1
fi

