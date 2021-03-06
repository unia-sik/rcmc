LIBRARY ieee;
USE ieee.std_logic_1164.all; 
use work.libeu.all;
ENTITY LZCounter32Bit IS 
	PORT
	(
		A :  IN  STD_LOGIC_VECTOR(31 DOWNTO 0);
		V :  OUT  STD_LOGIC;
		Z :  OUT  STD_LOGIC_VECTOR(4 DOWNTO 0)
	);
END LZCounter32Bit;
ARCHITECTURE Behavioral OF LZCounter32Bit IS 
signal w80: std_logic;
signal w2: std_logic;
signal w3: std_logic;
signal w4: std_logic;
signal w8: std_logic;
signal w114: std_logic;
signal w150: std_logic;
signal w151: std_logic;
signal w152: std_logic;
signal w153: std_logic;
signal w154: std_logic;
signal w155: std_logic;
signal w156: std_logic;
signal w157: std_logic;
signal w158: std_logic;
signal w159: std_logic;
signal w12: std_logic;
signal w14: std_logic;
signal w15: std_logic;
signal w16: std_logic;
signal w78: std_logic;
signal w146: std_logic;
signal w160: std_logic;
signal w148: std_logic;
signal w161: std_logic;
signal w162: std_logic;
signal w163: std_logic;
signal w164: std_logic;
signal w20: std_logic;
BEGIN
  w80 <= (A(7) OR (A(5) AND NOT w156)) OR ((A(3) OR (A(1) AND NOT w157)) AND NOT w150);
  w2 <= w163 OR (w164 AND NOT w152);
  w3 <= w4 AND NOT w151;
  w4 <= w161 OR (w162 AND NOT w153);
  w8 <= w153 AND NOT w151;
  w114 <= (w152 OR w8) OR ((w160 OR (w150 AND NOT w155)) AND NOT w154);
  Z(0) <= NOT((w14 OR w15));
  Z(2) <= NOT(w114);
  w150 <= w156 OR (A(5) OR A(4));
  w151 <= w152 OR (w164 OR (A(25) OR A(24)));
  w152 <= w163 OR (A(29) OR A(28));
  w153 <= w161 OR (A(21) OR A(20));
  w154 <= w151 OR (w153 OR (w162 OR (A(17) OR A(16))));
  Z(4) <= NOT(w154);
  w155 <= w160 OR (w159 OR (A(9) OR A(8)));
  w156 <= A(7) OR A(6);
  w157 <= A(3) OR A(2);
  w158 <= A(15) OR A(14);
  w159 <= A(11) OR A(10);
  w12 <= w155 OR (w150 OR (w157 OR (A(1) OR A(0))));
  w14 <= w146 OR (w148 AND NOT w151);
  w15 <= w16 AND NOT w154;
  w16 <= w78 OR (w80 AND NOT w155);
  V <= NOT((w154 OR w12));
  w78 <= (A(15) OR (A(13) AND NOT w158)) OR ((A(11) OR (A(9) AND NOT w159)) AND NOT w160);
  Z(1) <= NOT(((w2 OR w3) OR w20));
  Z(3) <= NOT((w151 OR (w155 AND NOT w154)));
  w146 <= (A(31) OR (A(29) AND NOT w163)) OR ((A(27) OR (A(25) AND NOT w164)) AND NOT w152);
  w160 <= w158 OR (A(13) OR A(12));
  w148 <= (A(23) OR (A(21) AND NOT w161)) OR ((A(19) OR (A(17) AND NOT w162)) AND NOT w153);
  w161 <= A(23) OR A(22);
  w162 <= A(19) OR A(18);
  w163 <= A(31) OR A(30);
  w164 <= A(27) OR A(26);
  w20 <= ((w158 OR (w159 AND NOT w160)) OR ((w156 OR (w157 AND NOT w150)) AND NOT w155)) AND NOT w154;
END Behavioral;
