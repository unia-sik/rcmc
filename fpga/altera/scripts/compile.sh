#!/bin/bash

QUARTUS_PATH="$HOME/my_intelFPGA/18.0/quartus/bin"

$QUARTUS_PATH/quartus_sh --flow compile $1
