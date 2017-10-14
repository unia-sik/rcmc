#!/bin/bash


# colours for messages
COLMSG='\033[1;30m'
COLSUCC='\033[0;32m'
COLFAIL='\033[0;31m'
COLNONE='\033[0m'


function run_tests_in_folder {
  # Run comparison on all tests in folder.
  for f in $1;
  do
    # Ignore tests for specific cids.
    if [[ $f =~ .*_cid[0-9]+.riscv.elf ]]
    then
      continue
    fi

    NAME=$(basename "$f")
    echo -n "Test $NAME"

    ./cmp4x4_one.sh "$f" $2 $3 1> /dev/null
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
}

if [ $# -eq 0 ]
then
  echo "Usage: $0 <test names>"
  echo "Available tests: riscv-tests fgmp mpi plansch"
  exit
fi

while [ $# -gt 0 ]
do
    case "$1" in
        riscv-tests)
        # Clean & build riscv-tests for rv64i.
        echo "Building $1..."
        make ARCH=rv64i -C "${RCMC}/baremetal/tests/riscv-tests/" clean all 1> /dev/null
        echo "Running $1..."
        run_tests_in_folder "${RCMC}/baremetal/tests/riscv-tests/build/rv64ui_*.riscv.elf"
        echo "Done with $1!"
        shift
        ;;

    fgmp)
      # Clean & build fgmp tests.
      echo "Building $1..."
      make -C "${RCMC}/baremetal/tests/4x4/" ARCH=rv64i clean all #1> /dev/null
      echo "Running $1..."
      run_tests_in_folder "${RCMC}/baremetal/tests/4x4/build/*.rv64i.elf"
      echo "Done with $1!"
      shift
      ;;

    mpi)
      # Clean & build mpi tests.
      echo "Building $1..."
      make -C "${RCMC}/baremetal/tests/mpi/" ARCH=rv64i clean all #1> /dev/null
      echo "Running $1..."
      run_tests_in_folder "${RCMC}/baremetal/tests/mpi/build/*.rv64i.elf" 100000 100
      echo "Done with $1!"
      shift
      ;;

    plansch)
      # Clean & build plansch tests.
      echo "Building $1..."
      cd "${RCMC}/baremetal/tests/fgmp/plansch/"
      rm ./*.riscv.elf
      "${RCMC}/baremetal/tests/fgmp/plansch/build_all.sh" 1> /dev/null
      cd "${RCMC}/baremetal/test_comparison/"
      echo "Running $1..."
      run_tests_in_folder "${RCMC}/baremetal/tests/fgmp/plansch/ocean_p16*.riscv.elf" 30000000 500
      echo "Done with $1!"
      shift
      ;;

    *)
      echo "Unknown tests '$1'"
      exit
      ;;
    esac
done
