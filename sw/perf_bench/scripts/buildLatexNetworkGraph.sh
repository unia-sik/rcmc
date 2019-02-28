#!/bin/bash

cat logs/app_sort_sync_with_cg/cg_4x4.macsim.log | grep "\-\->" | cut -d":" -f1 | 
while read line
do 
#	echo $line
	tmp=$(echo $line | cut -d" " -f1)
	x0=$(bc -l <<< "scale=0; $tmp % 4")
	y0=$((tmp / 4))

	tmp=$(echo $line | cut -d" " -f3)
	x1=$(bc -l <<< "scale=0; $tmp % 4")
	y1=$((tmp / 4))

	echo -n "\draw [->] ("
	echo -n $y0
	echo -n $x0
	echo -n ")"

	if [ "$y0" == "$y1" ]
	then
		echo -n "[out=-135, in=-45]"
	fi

	if [ "$x0" == "$x1" ]
	then
		echo -n "[out=-135, in=135]"
	fi

	echo -n " -- "

	y=$(echo $line | cut -d" " -f3)
	echo -n "("
	echo -n $y1
	echo -n $x1
	echo  ");"


done

