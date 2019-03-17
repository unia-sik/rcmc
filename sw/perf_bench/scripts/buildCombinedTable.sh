#!/bin/bash

for file in $(
	for file in $(ls logs/app_sort_sync_no_cg/*.log | xargs -n 1 basename )
        do 
		echo $(echo $file | egrep -o "[[:digit:]]+x[[:digit:]]+" | cut -d"x" -f1)~$file
        done | sort -g
)
do 


	
	echo -n "$file "
#	printf "%40s" "\\texttt{$(basename $(echo $file | cut -d"~" -f2) $(echo $file | grep -o "_[[:digit:]]*x[[:digit:]]*").macsim.log | sed 's/_/\\_/g')}  &"
#	printf "%8s  &" $(echo $file | grep -o "_[[:digit:]]*x[[:digit:]]*" | cut -d"_" -f2 | cut -d"x" -f1)
#	printf "%'20.f  &" "$(cat logs/app_sort_sync_no_cg/$(echo $file | cut -d"~" -f2) | grep "Max" | cut -d" " -f4)"
#	printf "%'20.f  &" "$(cat logs/app_sort_sync_no_cg/$(echo $file | cut -d"~" -f2) | grep "Min" | cut -d" " -f4)"
#	printf "%'20.f  &" "$(cat logs/app_sort_sync_no_cg/$(echo $file | cut -d"~" -f2) | grep "Total" | cut -d" " -f4)"
	a=$(cat logs/app_sort_async/$(echo $file | cut -d"~" -f2) | grep "Total" | cut -d" " -f4)


#	printf "%'20.f  &" "$(cat logs/app_sort_sync_with_cg/$(echo $file | cut -d"~" -f2) | grep "Max" | cut -d" " -f4)"
#	printf "%'20.f  &" "$(cat logs/app_sort_sync_with_cg/$(echo $file | cut -d"~" -f2) | grep "Min" | cut -d" " -f4)"
#	printf "%'20.f  &" "$(cat logs/app_sort_sync_with_cg/$(echo $file | cut -d"~" -f2) | grep "Total" | cut -d" " -f4)"

#	printf "%'20.f  &" "$(cat logs/app_sort_async/$(echo $file | cut -d"~" -f2) | grep "Max" | cut -d" " -f4)"
#	printf "%'20.f  &" "$(cat logs/app_sort_async/$(echo $file | cut -d"~" -f2) | grep "Min" | cut -d" " -f4)"
#	printf "%'20.f  &" "$(cat logs/app_sort_async/$(echo $file | cut -d"~" -f2) | grep "Total" | cut -d" " -f4)"
	
#	printf "%'20.f  &" "$(cat logs/$(echo $file | cut -d"~" -f2) | grep "Max" | cut -d" " -f4)"
#	printf "%'20.f  &" "$(cat logs/$(echo $file | cut -d"~" -f2) | grep "Min" | cut -d" " -f4)"
#	printf "%'20.f   " "$(cat logs/$(echo $file | cut -d"~" -f2) | grep "Total" | cut -d" " -f4)"
	b=$(cat logs/$(echo $file | cut -d"~" -f2) | grep "Total" | cut -d" " -f4)

	echo -n "$(echo $b) / $(echo $a) "       
	bc -l <<< "($(echo $b) / $(echo $a) - 1)*100"       
 
	echo "\\\\"
done

