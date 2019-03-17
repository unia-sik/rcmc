#!/bin/bash

rawInput=$(cat)
total=$(echo $rawInput | egrep -o "Total: [[:digit:]]+" | egrep -o "[[:digit:]]+")

echo "\begin{tabular}{l|r}"
echo "    Instruction & Cycles\\\\"
echo "    \hline"
echo "    \hline"
while read line
do 
    echo "    $(echo $line | cut -d"|" -f1) & $(printf "%'.f\n" $(echo -n $line | cut -d"|" -f2)y)\\\\";
done <<< $(echo $rawInput | egrep -o "[[:alpha:]]+\| [[:digit:]]+")
echo "    \hline"
echo "    Total & $(printf "%'.f\n" $total)"
\echo "\end{tabular}"
