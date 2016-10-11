LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;




ENTITY TestCore IS
    PORT (	
				Clk: IN std_logic;
				Rst: IN std_logic;
				InData:				IN  std_logic_vector(Data_Length-1 DOWNTO 0);
				InAddress:			IN  std_logic_vector((Address_Length_X + Address_Length_Y)*2-1 DOWNTO 0);
				InDataAvailable: 	IN std_logic;
				InStallSignal: 	IN std_logic;
				
				
				OutData: 				OUT std_logic_vector(Data_Length-1 DOWNTO 0);
				OutAddress: 			OUT std_logic_vector((Address_Length_X + Address_Length_Y)*2-1 DOWNTO 0);
				OutDataAvailable: 	OUT std_logic
	 
				);
END TestCore;




ARCHITECTURE Behavioral OF TestCore IS

SIGNAL tmp : std_logic_vector(Data_Length-1 DOWNTO 0);
SIGNAL add : std_logic_vector((Address_Length_X + Address_Length_Y)*2-1 DOWNTO 0);
BEGIN
	PROCESS (CLK,RST)	
	


	
	BEGIN	
		IF RST = '0' THEN
			tmp <= 	x"000000000000EBFA";
			add <= "1101";
		ELSIF rising_edge(CLK) THEN
			OutData <= 	std_logic_vector(unsigned(tmp) + 1);
			tmp <= std_logic_vector(unsigned(tmp) + 1);
			
			OutAddress <= 	std_logic_vector(unsigned(add) + 1);
			add <= std_logic_vector(unsigned(add) + 1);
			
		END IF;
	
	END PROCESS;
	



	
	
	OutDataAvailable <= '1';
	
END Behavioral;
