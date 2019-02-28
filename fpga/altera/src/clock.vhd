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
iclock <=	not iclock after 0.5 ns;
clk <= iclock;
end behaviour;
