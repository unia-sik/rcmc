#!/bin/bash

rawInput=$(cat)
total=$(echo $rawInput | egrep -o "Total: [[:digit:]]+" | egrep -o "[[:digit:]]+")

echo "\begin{tikzpicture}"
echo "    \startDrawPieChart{0, 0}{5};"
while read line
do 
    x=$(echo -n $line | cut -d"|" -f2)
    new=$(bc -l <<< "$x/$total*360")
    echo "    \drawPieChart{0, 0}{5}{$new}{white,nearly transparent}{black}{$(echo $line | cut -d"|" -f1)};";
done <<< $(echo $rawInput | egrep -o "[[:alpha:]]+\| [[:digit:]]+")
echo "\end{tikzpicture}"
