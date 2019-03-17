#/bin/bash

for file in $(find summary -maxdepth 1 -type f | sort)
do 
	echo $file
       	cat $file
       	cat $file | grep ":node_inst| " logs/$(grep "Full log" | cut -d" " -f3)/PNOO.fit.rpt
       	echo "-----------------------------------------"
done
