#!/bin/sh
# Run one netrace benchmarks with all router configuration

MACSIM=../../../macsim/build/macsim-seq

mkdir -p netrace

# Download traces if not yet done
if [ ! -e traces/x264_64c_simsmall.tra.bz2 ]
then
    ./download_traces.sh
fi



# Simulate one trace with all necessary router configurations
# $1 short benchmark name
# $2 trace file
allrouters () {
    NAME=$1
    FILE=$2
    for ROUTER in perfect minbd PUBSSRR PBBSS0R \
                  PNNGG00 PNUGG00 PNBGG00 PUNGG00 PUUGG00 PUBGG00 PBNGG00 PBUGG00 PBBGG00 \
                  PUUGS00 PUUSG00 PUUSS00 PUUGG0R PUUGGR0 PUUGGRR
    do
        ${MACSIM} -Anetrace -N64 -R${ROUTER} -a traces/${FILE} -g -r -q > netrace/${NAME}.${ROUTER}.log
    done
}



# Given is the average time, one run (of 26) takes approximately
#                                                         one / all
allrouters blackS  blackscholes_64c_simsmall.tra.bz2    #       6h
allrouters blackM  blackscholes_64c_simmedium.tra.bz2   # 1.5h  23h
allrouters fluidS  fluidanimate_64c_simsmall.tra.bz2    # 2h
allrouters swapL   swaptions_64c_simlarge.tra.bz2       # 3h
allrouters fluidM  fluidanimate_64c_simmedium.tra.bz2   # 4h
allrouters bodyL   bodytrack_64c_simlarge.tra.bz2       # 4.5h
allrouters blackL  blackscholes_64c_simlarge.tra.bz2    # 6h
allrouters dedupM  dedup_64c_simmedium.tra.bz2          # 6.5h
allrouters vipsM   vips_64c_simmedium.tra.bz2           # 6.5h
allrouters ferretM ferret_64c_simmedium.tra.bz2         # 10h
allrouters fluidL  fluidanimate_64c_simlarge.tra.bz2    # 11h
allrouters x264M   x264_64c_simmedium.tra.bz2           # 15h
allrouters cannM   canneal_64c_simmedium.tra.bz2        # 24h
allrouters x264S   x264_64c_simsmall.tra.bz2


