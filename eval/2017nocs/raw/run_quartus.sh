#!/bin/sh
# evaluation of NoC space requirements (fitter)

## paths
WORKDIR=work
RESULTS=../quartus  # relative to $WORKDIR
VHDL_PROJECT=../../../fpga/core1/nxn_altera_stratix_v
PROJECT_FILE=nxn_altera_stratix_v.qpf

## config arrays
#declare -A bypassx=([n]=CONF_BYPASS_NONE [u]=CONF_BYPASS_UNBUF [b]=CONF_BYPASS_BUF)
#declare -A bypassy=([n]=CONF_BYPASS_NONE [u]=CONF_BYPASS_UNBUF [b]=CONF_BYPASS_BUF)
#declare -A stallx=([c]=CONF_STALL_CHEAP [e]=CONF_STALL_EXP)
#declare -A stally=([c]=CONF_STALL_CHEAP [e]=CONF_STALL_EXP)
#declare -A injectx=([0]=CONF_INJECT_NONE [r]=CONF_INJECT_REQUEST) 
#declare -A injecty=([0]=CONF_INJECT_NONE [r]=CONF_INJECT_REQUEST)

## copy vhdl directory to local dir
mkdir -p $WORKDIR/prj
mkdir -p $WORKDIR/src
rsync -aq $VHDL_PROJECT/ $WORKDIR/prj/
rsync -aq $VHDL_PROJECT/../src/ $WORKDIR/src/

cd $WORKDIR/prj


#  for i in "${!bypassx[@]}"; do
#    for j in "${!bypassy[@]}"; do
#      for k in "${!stallx[@]}"; do
#        for l in "${!stally[@]}"; do
#          for m in "${!injectx[@]}"; do
#            for n in "${!injecty[@]}"; do

for WIDTH in 2 3 4 5 6 7 ; do
  for LOGCB in 1 2 3 4 5 ; do
    CB=`echo "2 ^ $LOGCB" | bc`
    for i in N U B ; do
      for j in N U B ; do
        for k in G S ; do
          for l in G S ; do
            for m in 0 R ; do
              for n in 0 R ; do

                # write NoC.chd and constatns.vhd
                ./config.sh $WIDTH $i$j$k$l$m$n $CB

                rm -rf output_files
 
                quartus_sh --flow compile $PROJECT_FILE

                # save quartus output
                OUT=$RESULTS/${WIDTH}x${WIDTH}/logcb${LOGCB}/P$i$j$k$l$m$n
                mkdir -p $OUT/
                rsync -aq output_files/ $OUT/ --exclude=riscv.sof
                cp constants.vhd $OUT/constants.vhd

              done
            done
          done
        done
      done
    done
  done
done
