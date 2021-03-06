#!/bin/sh
# Generate the file NoC.vhdl accoring to the network size

PORTTYPE_VERTICAL="P_PORT_VERTICAL"
PORTTYPE_VERTICAL_BACK="P_PORT_VERTICAL_BACK"
PORTTYPE_HORIZONTAL="P_PORT_HORIZONTAL"
PORTTYPE_HORIZONTAL_BACK="P_PORT_HORIZONTAL_BACK"
PORTTYPE_LOCAL="P_PORT_LOCAL"
ADDRESSTYPE="Address"
NODETYPE="NOCUNIT"
CLOCK="CLOCK_50"
RESET="SW(17)"




# $1 value
# $2 len
print_bin ()
{
    n="$1"
    bit=""
    len="$2"
    while [ "$len" -gt 0 ]
    do
        bit="$(( n&1 ))$bit";
        : $(( n >>= 1 ))
        : $(( len -= 1 ))
    done
    printf "$bit"
}



if [ $# -ne 2 ]
then
    echo "Usage: $0 <width> <height>"
    exit 1
fi

XMAX=$(( $1 - 1))
YMAX=$(( $2 - 1))


# How many bits are required? log_2($YMAX)
XBITS=1
max=2
while [ "$1" -gt "$max" ]
do
    max=$(( $max * 2 ))
    XBITS=$(( $XBITS + 1 ))
done

YBITS=1
max=2
while [ "$2" -gt "$max" ]
do
    max=$(( $max * 2 ))
    YBITS=$(( $YBITS + 1 ))
done








cat <<EOL
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use WORK.CONSTANTS.all;
use WORK.LIBNODE.all;

entity NoC is
  port(SW       : in  std_logic_vector(17 downto 0);
       CLOCK_50 : in  std_logic;
       LEDR     : out std_logic_vector(64 downto 0));
end;

architecture STRUCTURE of NoC is
  component NOCUNIT
    generic(id : integer; count : integer; nocdim : std_logic_vector(63 downto 0));
    port(Clk         : in  std_logic;
         rst_n       : in  std_logic;
         LED         : out std_logic;
         NorthIn     : in  P_PORT_VERTICAL_BACK;
         NorthOut    : out P_PORT_VERTICAL;
         SouthIn     : in  P_PORT_VERTICAL;
         SouthOut    : out P_PORT_VERTICAL_BACK;
         EastIn      : in  P_PORT_HORIZONTAL_BACK;
         EastOut     : out P_PORT_HORIZONTAL;
         WestIn      : in  P_PORT_HORIZONTAL;
         WestOut     : out P_PORT_HORIZONTAL_BACK;
         CoreAddress : in  Address
         );
  end component;


EOL


for y in $(seq 0 $YMAX)
do
    for x in $(seq 0 $XMAX)
    do
	printf "\n-------------------------------------------------"
	printf "\n--SIGNALS for NODE "
        print_bin $y $YBITS
        print_bin $x $XBITS
        printf "\n\n"

	printf "SIGNAL N_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "EAST_OUT\t\t\t:%s;\n" $PORTTYPE_HORIZONTAL

	printf "SIGNAL N_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "NORTH_OUT\t\t\t:%s;\n" $PORTTYPE_VERTICAL

	printf "SIGNAL N_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "WEST_OUT\t\t\t:%s;\n" $PORTTYPE_HORIZONTAL_BACK

	printf "SIGNAL N_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "SOUTH_OUT\t\t\t:%s;\n" $PORTTYPE_VERTICAL_BACK

	printf "SIGNAL N_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "CORE_ADDRESS\t\t\t:%s;\n" $ADDRESSTYPE
    done
done

printf "\n\n\n\nbegin\n"

YBASE=`echo 2 ^ $XBITS | bc`
NNODES=$(( $1 * $2 ))

for y in $(seq 0 $YMAX)
do
    for x in $(seq 0 $XMAX)
    do
        ID=$(( $y * $YBASE + $x ))

        printf "\t$NODETYPE"
        print_bin $y $YBITS
        print_bin $x $XBITS
        printf " : $NODETYPE\n"

        printf "generic map ( id => $ID, count => $NNODES, nocdim => x\"00010001%04x%04x\" )\n" $2 $1

	printf " port map( "
	printf "\n\t\t\t\t$CLOCK,\n\t\t\t\t$RESET,\n"

	printf "\t\t\t\tLEDR($ID),\n"

	printf "\t\t\t\tN_"
	print_bin $(( ($y+$YMAX) % ($YMAX+1) )) $YBITS
	print_bin $x $XBITS
	printf "SOUTH_OUT,\n"

	printf "\t\t\t\tN_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "NORTH_OUT,\n"

	printf "\t\t\t\tN_"
	print_bin $(( ($y+1) % ($YMAX+1) )) $YBITS
	print_bin $x $XBITS
	printf "NORTH_OUT,\n"

	printf "\t\t\t\tN_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "SOUTH_OUT,\n"

	printf "\t\t\t\tN_"
	print_bin $y $YBITS
	print_bin $(( ($x+1) % ($XMAX+1) )) $XBITS
	printf "WEST_OUT,\n"

	printf "\t\t\t\tN_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "EAST_OUT,\n"

	printf "\t\t\t\tN_"
	print_bin $y $YBITS
	print_bin $(( ($x+$XMAX) % ($XMAX+1) )) $XBITS
	printf "EAST_OUT,\n"
	
	printf "\t\t\t\tN_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "WEST_OUT,\n"

	printf "\t\t\t\tN_"
        print_bin $y $YBITS
        print_bin $x $XBITS
	printf "CORE_ADDRESS);\n"
    done
done

printf "\n\n\n\n"
printf "\n\n\n\n"

for y in $(seq 0 $YMAX)
do
    for x in $(seq 0 $XMAX)
    do
        printf "\tN_"
        print_bin $y $YBITS
        print_bin $x $XBITS
        printf "CORE_ADDRESS.X <= \""
        print_bin $x $XBITS
        printf "\";\n\tN_"
        print_bin $y $YBITS
        print_bin $x $XBITS
        printf "CORE_ADDRESS.Y <= \""
        print_bin $y $YBITS
        printf "\";\n"
    done
done

printf "end; \n\n"
