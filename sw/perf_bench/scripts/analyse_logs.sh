#!/bin/bash

declare -A min
declare -A max
declare -A total


for file in $(find logs/ -maxdepth 1 -type f |awk -F'[_/]' '{print $(NF-0), $0}'|sort -n|sed 's/.* //');
do
# 	echo $file;
    index=$(basename $file .macsim.log | sed "s/[[:digit:]]*x[[:digit:]]*//g")
    dim=$(echo $file | grep -o "[[:digit:]]*x[[:digit:]]*" | cut -d"x" -f1)
    
    
    min[$index]="${min[$index]}$(echo "($dim, $(cat $file | grep "Min. Execution Time:" | grep -o "[[:digit:]]*"))")"
    max[$index]="${max[$index]}$(echo "($dim, $(cat $file | grep "Max. Execution Time:" | grep -o "[[:digit:]]*"))")"
    total[$index]="${total[$index]}$(echo "($dim, $(cat $file | grep "Total Execution Time:" | grep -o "[[:digit:]]*"))")"
    
done;

for i in "${!min[@]}"
do
    echo "%$i min"
	echo "\addplot +[mark=*] coordinates {"
    echo ${min[$i]}
	echo "};"
done

for i in "${!max[@]}"
do
    echo "%$i max"
	echo "\addplot +[mark=*] coordinates {"
    echo ${max[$i]}
	echo "};"
done

for i in "${!total[@]}"
do
    echo "%$i total"
	echo "\addplot +[mark=*] coordinates {"
    echo ${total[$i]}
	echo "};"
done



