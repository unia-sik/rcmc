#!/bin/bash

#file=$(cat $1 | grep "Full log:" | cut -d" " -f3)
#file="output_files"


startLine=$(cat $1 | grep -n "Fitter Resource Utilization by Entity" | tail -n1 | grep -o "[[:digit:]]*")
numLine=$(cat $1 | tail -n +$startLine | egrep -n "+----------------------------" | grep -o "[[:digit:]]*" | head -n3 | tail -n1)

nocunitBoundary=$(cat $1 | tail -n +$startLine | head -n$numLine | egrep -on "; *\|nocunit:" | grep -o "[[:digit:]]*" | head -n2)

nocunitStart=$(echo $nocunitBoundary | cut -d" " -f1)
nocunitEnd=$(echo $nocunitBoundary | cut -d" " -f2)

cat $1 | tail -n +$(( $startLine + $nocunitStart - 5)) | head -n$(( $nocunitEnd - $nocunitStart + 4)) 


