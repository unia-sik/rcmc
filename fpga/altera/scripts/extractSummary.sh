#!/bin/bash

declare -A val_alm
declare -A val_reg
declare -A val_bit

for file in $(find summary -maxdepth 1 -type f |awk -F'[_/]' '{print $(NF-1), $0}'|sort -n|sed 's/.* //')
do
#       	echo $file
	dimReal=$(echo $file | grep -o "_[[:digit:]]*\." | grep -o "[[:digit:]]*")
	dim=$(echo $file | grep -o "[[:digit:]]*x[[:digit:]]*" | cut -d"x" -f1)

	index=$(echo $file | cut -d"/" -f2 | cut -d"." -f1 | sed "s/${dim}x${dim}_[[:digit:]]*//g")

	val_alm[$index]="${val_alm[$index]}$(echo "($dim, $(bc -l <<< "scale=3; $(cat $file | grep "ALMs" | cut -d" " -f6 | sed "s/,//g") / ($dimReal * $dimReal)"))")"
	val_reg[$index]="${val_reg[$index]}$(echo "($dim, $(bc -l <<< "scale=3; $(cat $file | grep "Total registers" | cut -d" " -f4 | sed "s/,//g") / ($dimReal * $dimReal)"))")"
	val_bit[$index]="${val_bit[$index]}$(echo "($dim, $(bc -l <<< "scale=3; $(cat $file | grep "memory bits" | cut -d" " -f6 | sed "s/,//g") / ($dimReal * $dimReal)"))")"



done

for i in "${!val_alm[@]}"
do
	echo "% $i alm";
	echo "\addplot +[mark=*] coordinates {"
	echo ${val_alm[$i]}
	echo "};"
done

for i in "${!val_reg[@]}"
do
	echo "% $i reg";
	echo "\addplot +[mark=*] coordinates {"
	echo ${val_reg[$i]}
	echo "};"
done

for i in "${!val_bit[@]}"
do
	echo "% $i bit";
	echo "\addplot +[mark=*] coordinates {"
	echo ${val_bit[$i]}
	echo "};"
done



