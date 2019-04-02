#!/bin/sh
# Configure an RC/MC manycore NoC with One-To-One router
#
# This script should be called from a temporary directory, where all the
# configuration files are written. These are:
#
# constants.vhd         Constants depending on the NoC size
# Noc.vhd               Wiring for connecting the nodes
# dmem_ghdl.vhd         Preinitialised memory
# ghdl_analyze.sh       Script to analyse the manycore with GHDL
# ghdl_convert.sh       Script to convert GHDL output (.vcd) to MacSim regdump
# clock.vhd             Clock signal for GHDL


if [ $# -ne 5 ]
then
    echo "Usage: $0 <width> <router> <corner buffer size> <memory size> <memory dump>"
    exit 1
fi

MYDIR=$(dirname $0)

$MYDIR/gen_constants.sh $1 UBGG00 $3 > constants.vhd
#$MYDIR/gen_noc.sh $1 $1 > NoC.vhd
#$MYDIR/../../tools/shexdump/shexdump -M $5 1 $4 > dmem_ghdl.vhd
$MYDIR/../../tools/shexdump/shexdump -V $5 1 $4 > dmem_ghdl.vhd

$MYDIR/gen_ghdl.sh > ghdl_analyze.sh
chmod 775 ghdl_analyze.sh

echo "#!/bin/sh" > ghdl_convert.sh
echo "awk -v drift=3 cid=generate -f vcd2regdump.awk \$1" >> ghdl_convert.sh
chmod 775 ghdl_convert.sh


cat <<EOL > clock.vhd
LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY sim_clock IS
	PORT
	(
		rst_n : out  std_logic;
		clk   : out  std_logic
	);
END sim_clock;

ARCHITECTURE behaviour OF sim_clock IS
	SIGNAL ireset : std_logic := '0';
	SIGNAL iclock : std_logic := '0';

begin
RESET:
ireset <=	'1' after 0.5 ns when ireset = '0';
rst_n <= ireset;
CLOCK:
iclock <=	'1' after 0.5 ns when iclock = '0' else
			'0' after 0.5 ns when iclock = '1';
clk <= iclock;
end behaviour;
EOL
