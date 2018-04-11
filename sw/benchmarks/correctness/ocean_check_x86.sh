#!/bin/sh
# Check ocean with different processor numbers
#
# Standard test: ./ocean_check_x86.sh --pth 1 2 4 8 16 32 --mpi 1 2 4 8 16 32 64


# compile
cd ..
make build/ocean.x86 build/ocean.mpi.x86
cd correctness

MPI=0
PROBLEM_SIZE=9

if [ $# -eq 0 ]
then
    ../build/ocean.x86 8 "$PROBLEM_SIZE" | grep iter | diff correct.$PROBLEM_SIZE -
else
    for p in "$@"
    do
	if [ $p = "--pth" ]
	then
	    MPI=0
	elif [ $p = "--mpi" ]
	then
	    MPI=1
	elif [ -z "${p##*[!0-9]*}" ]
	then
	    echo "Number expected for problem size"
	    exit 1
	else
            if [ $MPI -eq 1 ]
            then
                mpirun -np $p ../build/ocean.mpi.x86 $p $PROBLEM_SIZE > out.tmp
            else
                ../build/ocean.x86 $p $PROBLEM_SIZE > out.tmp
            fi
            grep iter out.tmp | diff ocean.correct.$PROBLEM_SIZE -
        fi
    done
fi

