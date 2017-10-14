#!/bin/sh

BUILD_DIR="build"

for f in $BUILD_DIR/4x4_*.rv64i.elf
do
#    # Ignore tests for specific cids.
#    if [[ $f =~ .*_cid[0-9]+.rv64i.elf ]]
#    then
#      continue
#    fi

    NAME=$(basename "$f" .rv64i.elf)

    # Ignore benchmarks that don't work yet
    case $NAME in
        rv64ui_fence_i)         continue ;; # unsupported instruction
        4x4_fgmp_random)        continue ;; # Modelsim error 4OC
        mpi_alltoallv)          continue ;;
        mpi_example)            continue ;; # Modelsim error 4
        mpi_reduce)             continue ;; # Modelsim error 4
        mpi_scatter)            continue ;;
        mpi_scatterv)           continue ;; # MacSim problem
        ocean_p16n3)            continue ;; # MacSim problem
    esac


    echo -n "Test $NAME"

    ./cmp_one.sh "$f" $2 $3 1> /dev/null
    result=$?
    if [ $result -eq 0 ]
    then
        echo " passed."
    elif [ $result -eq 1 ]
    then
        echo " has diffs!"
    elif [ $result -eq 2 ]
    then
        echo " failed!"
    fi
done 

