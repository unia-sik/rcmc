#!/bin/bash
# run all tests


for i in {2..16}
do 
	for file in $(ls bin/*$1_test.elf)
	do 
		echo $file;
		echo $i
		 ../../macsim/build/macsim-seq -Ariscv -N${i}x${i} -Rpnoo -a $file -g -q | grep "exited"

		echo "";
	done
done

