LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY sim_reset IS
	PORT
	(
		rst_n   : out  std_logic
	);
END sim_reset;

ARCHITECTURE behaviour OF sim_reset IS
	SIGNAL ireset : std_logic := '0';

begin
RESET:
ireset <=	'1' after 0.5 ns when ireset = '0';
rst_n <= ireset;
end behaviour;