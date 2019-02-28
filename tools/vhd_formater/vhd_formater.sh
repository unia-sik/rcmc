#!/bin/bash

if (( $# == 0 ))
then
	echo "Usage $0 <path_to_vhdl_files>"
	exit
fi


for file in $(ls $1/*.vhd)
do
	emacs -batch $file --eval '(vhdl-beautify-buffer)' -f save-buffer
done
