#!/bin/sh
# Generate the file constants.vhd accoring to the network size an router configuration


# $1 letter for bypass configuration
print_bypass ()
{
    case "$1" in
        N)  printf "CONF_BYPASS_NONE" ;;
        U)  printf "CONF_BYPASS_UNBUF" ;;
        B)  printf "CONF_BYPASS_BUF" ;;
        *)  echo "Unknown bypass configuration" 1>&2
            exit 1
    esac
}


# $1 letter for stall configuration
print_stall ()
{
    case "$1" in
        G)  printf "CONF_STALL_CHEAP" ;;
        S)  printf "CONF_STALL_EXPENSIVE" ;;
        *)  echo "Unknown stall configuration" 1>&2
            exit 1
    esac
}


# $1 letter for injection configuration
print_inject ()
{
    case "$1" in
        0)  printf "CONF_INJECT_NONE" ;;
        R)  printf "CONF_INJECT_REQUEST" ;;
        *)  echo "Unknown inject configuration" 1>&2
            exit 1
    esac
}





if [ $# -ne 3 ]
then
    echo "Usage: $0 <width> <router> <corner buffer entries>"
    exit 1
fi


cat <<EOL
library ieee;
use ieee.std_logic_1164.all;
use IEEE.math_real.all;

package constants is
  --Data Bits
  constant Data_Length : natural := 64;

  --Width(Height) of Network
EOL

echo "  constant Dimension : natural := $1;"

cat <<EOL
  --Address Bits for X Direction
  constant Address_Length_X : natural := integer(ceil(log2(real(Dimension))));
  --Address Bits for Y Direction
  constant Address_Length_Y : natural := integer(ceil(log2(real(Dimension))));

EOL

echo "  constant Corner_Buffer_Size : natural := $3;"

cat <<EOL

  --Relevant for Buffer
  constant NodeCount    : natural := Dimension*Dimension;
  constant HeapSize     : natural := 32;
  constant HeapSizeBits : natural := integer(ceil(log2(real(HeapSize))));

  constant FifoSendBufferSize : natural := 8;

  -- Constant types
  type Bypass is (CONF_BYPASS_NONE, CONF_BYPASS_UNBUF, CONF_BYPASS_BUF);
  type Stall is (CONF_STALL_CHEAP, CONF_STALL_EXP);
  type Inject is (CONF_INJECT_NONE, CONF_INJECT_REQUEST, CONF_INJECT_ALTERNATE, CONF_INJECT_THROTTLE);

  -- constants
EOL


printf "  constant conf_bypass_y : Bypass := "
print_bypass `echo $2 | cut -c2`
printf ";\n  constant conf_bypass_x : Bypass := "
print_bypass `echo $2 | cut -c3`
printf ";\n  constant conf_stall_y  : Stall  := "
print_stall `echo $2 | cut -c4`
printf ";\n  constant conf_stall_x  : Stall  := "
print_stall `echo $2 | cut -c5`
printf ";\n  constant conf_inject_y : Inject := "
print_inject `echo $2 | cut -c6`
printf ";\n  constant conf_inject_x : Inject := "
print_inject `echo $2 | cut -c7`
printf ";\nend constants;\n"

