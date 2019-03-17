#!/bin/bash

start=$(cat $1 | grep -on "Switched to core $2\." | grep -o "[[:digit:]]*:" | grep -o "[[:digit:]]*")
end=$(cat $1 | grep -on "Switched to core $(( $2 + 1 ))\." | grep -o "[[:digit:]]*:" | grep -o "[[:digit:]]*")

cat $1 | head -n+$(( end - 1 )) | tail -n+$start
