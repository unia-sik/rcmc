#!/bin/sh
# Generate a script that runs the GHDL analysis on all source files

# #1: relative path
abs_path() {
  echo "$(cd "$1" && pwd)"
#" just for syntax highlightning
}


MYDIR=$(dirname $0)
GHDL="$(abs_path ${MYDIR}/../../tools/ghdl/bin)/ghdl"
VHDL_PATH="$(abs_path ${MYDIR}/src)"
FLAGS="--ieee=synopsys"
MYDIR="$(abs_path ${MYDIR})"




cat <<EOL
#!/bin/sh
EOL

echo ${GHDL} -a ${FLAGS} constants.vhd

for FILE in LibNode libproc libeu cpu_top csrfile d_stage e_stage eu f_stage \
  m_stage regfile_ram_replacement regfile w_stage
do
    echo ${GHDL} -a ${FLAGS} ${VHDL_PATH}/${FILE}.vhd
done

for FILE in  NetworkInterfaceReceive nocunit Node
do
    echo ${GHDL} -a ${FLAGS} ${VHDL_PATH}/${FILE}.vhd
done

#for FILE in lzc161 lzc32 lzc53 lzc64 Division FPU FPUCLASS FPUCOMP FPUDIV \
#  FPUMINMAX FPUSIGNJ FPUSQRT FPUtoEFPU FPUtoFPU FPUtoINT INTtoFPU FPUFMA \
#  MultiplicationStage fregfile fregfile_ram_replacement mcodecache mcodefile
#do
#    echo ${GHDL} -a ${FLAGS} ${VHDL_PATH}/${FILE}.vhd
#done

echo ${GHDL} -a ${FLAGS} dmem_ghdl.vhd
echo ${GHDL} -a ${FLAGS} NoC.vhd
echo ${GHDL} -a ${FLAGS} clock.vhd

# Cycle difference between MacSim and GHDL simulation
echo export GHDL_CYCLE_DRIFT=8
echo export GHDL_COREID=nocunit

