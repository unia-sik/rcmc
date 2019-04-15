#!/bin/sh
# Generate a script that runs the GHDL analysis on all source files

# #1: relative path
abs_path() {
  echo "$(cd "$1" && pwd)"
#" just for syntax highlightning
}


MYDIR=$(dirname $0)
#GHDL="$(abs_path ${MYDIR}/../../tools/ghdl/bin)/ghdl"
VHDL_PATH="$(abs_path ${MYDIR}/src)"
#FLAGS="--ieee=synopsys"
MYDIR="$(abs_path ${MYDIR})"


cat <<EOL
#!/bin/sh
EOL

echo \${GHDL} -a \${GHDL_FLAGS} constants.vhd
echo \${GHDL} -a \${GHDL_FLAGS} clock.vhd

# common files
for FILE in LibNode libproc libeu
do
    echo \${GHDL} -a \${GHDL_FLAGS} ${VHDL_PATH}/${FILE}.vhd
done

# integer pipeline
for FILE in cpu_top csrfile d_stage e_stage eu f_stage \
  m_stage regfile_ram_replacement regfile w_stage
do
    echo \${GHDL} -a \${GHDL_FLAGS} ${VHDL_PATH}/${FILE}.vhd
done

# network interface (not required for single core, but don't hurt)
for FILE in NetworkInterfaceSend NetworkInterfaceRecv flit_buffer \
    flit_buffer2 top_level nocunit pnoo_node pnoo_node_double_corner \
    pnoo_node_sort pnoo_node_empty pnoo_node_no_rdy pnoo_node_srr \
    pnoo_node_drr pnoo_node_cg
do
    echo \${GHDL} -a \${GHDL_FLAGS} ${VHDL_PATH}/${FILE}.vhd
done

echo \${GHDL} -a \${GHDL_FLAGS} dmem_ghdl.vhd

echo export GHDL_CYCLE_DRIFT=3
echo export GHDL_COREID=generate
