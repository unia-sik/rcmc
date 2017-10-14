#!/bin/sh
# Configure an RC/MC manycore NoC by writing Noc.vhd and constants.vhd
#
# TODO: also configure memory size and dump

if [ $# -ne 3 ]
then
#    echo "Usage: $0 <width> <height> <router> <memory size> <memory dump>"
    echo "Usage: $0 <width> <router> <corner buffer size>"
    exit 1
fi


./gen_noc.sh $1 $1 > NoC.vhd
./gen_constants.sh $1 $2 $3 > constants.vhd
