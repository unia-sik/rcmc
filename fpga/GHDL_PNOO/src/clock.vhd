LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY sim_clock IS
	PORT
	(
		clk   : out  std_logic
	);
END sim_clock;

ARCHITECTURE behaviour OF sim_clock IS
	SIGNAL iclock : std_logic := '0';

begin
CLOCK:
iclock <=	'1' after 0.5 ns when iclock = '0' else
			'0' after 0.5 ns when iclock = '1';
clk <= iclock;
end behaviour;