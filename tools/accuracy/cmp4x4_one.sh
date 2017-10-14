#!/bin/sh
# Compare the execution of one benchmark with MacSim and GHDL
#
# Usage: cmp1x1_one.sh
#

GHDL="$(pwd)/../ghdl/bin/ghdl"
MACSIM="$(pwd)/../../macsim/build/macsim-seq"
ELF2HEX="$(pwd)/../../baremetal/arch/riscv/tools/riscv-elf2hex.x"
VHDL_PATH="$(pwd)/../../fpga/core1_4x4_ghdl_array"
HERE="$(pwd)"

MEMORY_SIZE=131072


# Color for messages.
COL_MSG='\033[1;30m'
COL_SUCCESS='\033[0;32m'
COL_FAIL='\033[0;31m'
COL_NONE='\033[0m'





if [ ! -e "$1" ]
then
  echo -e "${COL_MSG}ELF file not found!${NO_COLOR}" 1>&2
  exit 2
fi



# MacSim
echo "${COL_MSG}Running macsim-seq simulation${COL_NONE}"
${MACSIM} -A riscv -N4x4 -RCubcc00 -a $1 -y macsim.regdump -q 1> macsim.log
MACSIM_CYCLES=`grep "Execution Time" macsim.log | awk '{ print $3 }'`
echo "  ${COL_MSG}simulation time: ${COL_SUCCESS}${MACSIM_CYCLES} cycles${COL_NONE}"




echo "${COL_MSG}Running GHDL simulation${COL_NONE}"

# convert ELF binary to Intel hex format
$ELF2HEX -v $1 4 $MEMORY_SIZE > $VHDL_PATH/dmem.hex


# convert hex file to VHDL code of an initialised memory
cd $VHDL_PATH
python3 build_dmem.py -i dmem.hex


# build GHDL simulation
rm -rf work
mkdir work
${GHDL} -a --workdir=work constants.vhd
${GHDL} -a --workdir=work LibNode.vhd
${GHDL} -a --workdir=work libproc.vhd
${GHDL} -a --workdir=work libeu.vhd
${GHDL} -a --workdir=work cpu_top.vhd
${GHDL} -a --workdir=work csrfile.vhd
${GHDL} -a --workdir=work d_stage.vhd
${GHDL} -a --workdir=work dmem_replacementPY.vhd
${GHDL} -a --workdir=work --ieee=synopsys Multiplication.vhd
${GHDL} -a --workdir=work --ieee=synopsys Division.vhd
${GHDL} -a --workdir=work --ieee=synopsys Multiplexer.vhd
${GHDL} -a --workdir=work --ieee=synopsys LZCounter24Bit.vhd
${GHDL} -a --workdir=work --ieee=synopsys LZCounter32Bit.vhd
${GHDL} -a --workdir=work --ieee=synopsys LZCounter53Bit.vhd
${GHDL} -a --workdir=work --ieee=synopsys LZCounter64Bit.vhd
${GHDL} -a --workdir=work --ieee=synopsys LZCounter161Bit.vhd
${GHDL} -a --workdir=work --ieee=synopsys LZAnticipator161Bit.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPU.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPURegSet.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPUForward.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPUMux.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPUCOMP.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPUCLASS.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPUMINMAX.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPUSIGNJ.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPUtoFPU.vhd
${GHDL} -a --workdir=work --ieee=synopsys FPUtoINT.vhd
${GHDL} -a --workdir=work --ieee=synopsys INTtoFPU.vhd
${GHDL} -a --workdir=work --ieee=synopsys FMADD.vhd
${GHDL} -a --workdir=work e_stage.vhd
${GHDL} -a --workdir=work eu.vhd
${GHDL} -a --workdir=work f_stage.vhd
${GHDL} -a --workdir=work m_stage.vhd
${GHDL} -a --workdir=work NetworkInterfaceReceive.vhd
${GHDL} -a --workdir=work NetworkInterfaceSend.vhd
${GHDL} -a --workdir=work NoC.vhd
${GHDL} -a --workdir=work nocunit.vhd
${GHDL} -a --workdir=work Node.vhd
${GHDL} -a --workdir=work regfile_ram_replacement.vhd
${GHDL} -a --workdir=work regfile.vhd
${GHDL} -a --workdir=work w_stage.vhd

${GHDL} -a --workdir=work clock.vhd
${GHDL} -a --workdir=work reset.vhd

${GHDL} -e --workdir=work NoC


# run GHDL simulation
${GHDL} -r --workdir=work NoC --stop-time=$(($MACSIM_CYCLES+4))ns --vcd=ghdl.vcd 2> /dev/null

# convert to register dump
python3 analyze_vcd.py -i ghdl.vcd
mv ghdl.register.log $HERE/ghdl.regdump

cd $HERE


# comparison
if cmp --silent macsim.regdump ghdl.regdump
then
  echo "${COL_SUCCESS}Register dumps are equal!${COL_NONE}"
  exit 0
else
  echo "${COL_FAIL}Register dumps differ:${COL_NONE}"
  diff macsim.regdump ghdl.regdump
  exit 1
fi


