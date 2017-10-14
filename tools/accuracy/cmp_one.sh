#!/bin/sh
# Compare the execution of one benchmark with MacSim and GHDL
#
# Usage: cmp_one.sh
#

GHDL="$(pwd)/../ghdl/bin/ghdl"
MACSIM="$(pwd)/../../macsim/build/macsim-seq"
SHEXDUMP="$(pwd)/../shexdump/shexdump"
VHDL_PATH="$(pwd)/../../fpga/core1/src"

MEMORY_SIZE=131072
#MEMORY_SIZE=524288
# TODO: ghdl crashes with larger memory


# Color for messages.
COL_MSG='\033[1;30m'
COL_SUCCESS='\033[0;32m'
COL_FAIL='\033[0;31m'
COL_NONE='\033[0m'


TIME_START=$(date +%s)



if [ ! -e "$1" ]
then
  echo -e "ELF file not found!" 1>&2
  exit 2
fi
rm -rf work
mkdir work



# MacSim
printf "MacSim simulation"
${MACSIM} -A riscv -N4x4 -RPUBGG00 -a $1 -y work/macsim.regdump -q 1> work/macsim.log
MACSIM_CYCLES=$(grep "Execution Time" work/macsim.log | awk '{ print $3 }')
TIME_MACSIM=$(date +%s)
echo "${COL_MSG} ($((TIME_MACSIM-TIME_START)) seconds)${COL_NONE} simulated time: ${COL_MSG}${MACSIM_CYCLES} cycles${COL_NONE}"



# GHDL analysis
printf "GHDL analysis"
$SHEXDUMP -M $1 1 $MEMORY_SIZE > work/dmem_ghdl.vhd

cd work
${GHDL} -a ../vhdl/constants.vhd
${GHDL} -a ${VHDL_PATH}/LibNode.vhd
${GHDL} -a ${VHDL_PATH}/libproc.vhd
${GHDL} -a ${VHDL_PATH}/libeu.vhd
${GHDL} -a ${VHDL_PATH}/cpu_top.vhd
${GHDL} -a ${VHDL_PATH}/csrfile.vhd
${GHDL} -a ${VHDL_PATH}/d_stage.vhd
${GHDL} -a dmem_ghdl.vhd
${GHDL} -a ${VHDL_PATH}/e_stage.vhd
${GHDL} -a ${VHDL_PATH}/eu.vhd
${GHDL} -a ${VHDL_PATH}/f_stage.vhd
${GHDL} -a ${VHDL_PATH}/m_stage.vhd
${GHDL} -a ${VHDL_PATH}/NetworkInterfaceReceive.vhd
${GHDL} -a ${VHDL_PATH}/NetworkInterfaceSend.vhd
${GHDL} -a ../vhdl/NoC.vhd
${GHDL} -a ${VHDL_PATH}/nocunit.vhd
${GHDL} -a ${VHDL_PATH}/Node.vhd
${GHDL} -a ../vhdl/regfile_ram_replacement.vhd
${GHDL} -a ../vhdl/regfile.vhd
${GHDL} -a ${VHDL_PATH}/w_stage.vhd

${GHDL} -a ../vhdl/clock.vhd
${GHDL} -a ../vhdl/reset.vhd

${GHDL} -e NoC


# run GHDL simulation
TIME_ANALYSIS=$(date +%s)
echo "${COL_MSG} ($((TIME_ANALYSIS-TIME_MACSIM)) seconds)${COL_NONE}"
printf "GHDL simulation"
${GHDL} -r NoC --stop-time=$(($MACSIM_CYCLES+9))ns --vcd=ghdl.vcd 2> /dev/null
cd ..


# convert to register dump
TIME_SIM=$(date +%s)
echo "${COL_MSG} ($((TIME_SIM-TIME_ANALYSIS)) seconds)${COL_NONE}"
printf "Converting VCD to regdump"
awk -f vcd2regdump.awk work/ghdl.vcd > work/ghdl.regdump
TIME_CONV=$(date +%s)
echo "${COL_MSG} ($((TIME_CONV-TIME_SIM)) seconds)${COL_NONE}"



# comparison
if cmp --silent work/macsim.regdump work/ghdl.regdump
then
  echo "${COL_SUCCESS}Register dumps are equal!${COL_NONE}"
  exit 0
else
  echo "${COL_FAIL}Register dumps DIFFER!${COL_NONE}"
#  diff work/macsim.regdump work/ghdl.regdump
  exit 1
fi


