#!/bin/bash
# Convert the raw data into tables for the tikz diagrams
#
# Input:
# raw/netrace/
# raw/patterns.csv
# raw/quartus/
#
# Output:
# results/*.dat


RESULTS=results

NETRACE=netrace.csv
PATTERNS=raw/patterns.csv
QUARTUS=quartus.csv




# raw/netrace/ -> netrace.csv

#echo "benchmark,router,cycles" > $NETRACE
#for BENCHMARK in blackL blackM blackS bodyL cannM dedupM ferretM fluidL fluidM fluidS swapL vipsM x264M x264S
#do
#    for ROUTER in perfect minbd PUBSSRR PBBSS0R \
#                  PNNGG00 PNUGG00 PNBGG00 PUNGG00 PUUGG00 PUBGG00 PBNGG00 PBUGG00 PBBGG00 \
#                  PUUGS00 PUUSG00 PUUSS00 PUUGG0R PUUGGR0 PUUGGRR
#    do
#        FILENAME=$BENCHMARK.P$ROUTER.log
#        if [ -e $FILENAME ]
#        then
#            CYCLES=`awk '/Execution Time: / {print $3}' $FILENAME`
#        else
#            CYCLES=0
#        fi
#        echo "$BENCHMARK,$ROUTER,$CYCLES" >> $NETRACE
#    done
#done




# raw/quartus -> quartus.csv
# size of router in ALMs

echo > quartus.tmp
for WIDTH in 2 3 4 5 6 7 8 ; do
    for LOGCB in 1 2 3 4 5 ; do
        CB=`echo "2 ^ $LOGCB" | bc`
        filename="raw/quartus/${WIDTH}x${WIDTH}/${WIDTH}x${WIDTH}_ap_${CB}cb_seed1_fit_node.csv"
        if [ -e $filename ]
        then
          tail -n +2 $filename | sed -e "s|^|$WIDTH,$LOGCB,|" >> quartus.tmp
        fi
    done
done
echo "width,cb,ybypass,xbypass,ystall,xstall,yinj,xinj,alms" > $QUARTUS
awk -f scripts/quartus.awk quartus.tmp | sort | sort -k3,3 >> $QUARTUS
rm quartus.mp









mkdir -p $RESULTS

awk -f scripts/bypass_area.awk $QUARTUS  > $RESULTS/bypass_area.dat
awk -f scripts/area.awk        $QUARTUS  > $RESULTS/area.dat
awk -f scripts/bypass.awk      $PATTERNS > $RESULTS/bypass.dat
awk -f scripts/nt_bypass.awk   $NETRACE  > $RESULTS/nt_bypass.dat
awk -f scripts/nt_stall.awk    $NETRACE  > $RESULTS/nt_stall.dat
awk -f scripts/nt_fairinj.awk  $NETRACE  > $RESULTS/nt_fairinj.dat
awk -f scripts/scale_area.awk  $QUARTUS  > $RESULTS/scale_area.dat
awk -f scripts/scale.awk       $PATTERNS > $RESULTS/scale.dat
awk -f scripts/scale_total.awk $PATTERNS > $RESULTS/scale_total.dat
awk -f scripts/netrace.awk     $NETRACE | sort > $RESULTS/netrace.dat

# unused in NOCS 2017 paper:
awk -f scripts/stall.awk       $PATTERNS > $RESULTS/stall.dat

