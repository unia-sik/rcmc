#!/bin/bash

# if [ "$#" -eq 0 ]
# then
#     echo "Usage: $0 <elf-file>"
#     exit 0
# fi
# 
# ./configure $1
ghdl  -r --workdir=build/ghdl top_level --stop-time=1700ns --wave=simulation/noc_2x2.ghw

