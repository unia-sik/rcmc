library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;
use work.libeu.all;


entity FPUSIGNJ is
    Port ( 
			A : in std_logic_vector(63 downto 0);
			B : in std_logic_vector(63 downto 0);
			Funct : in std_logic_vector(6 downto 0);
			RM : in std_logic_vector(2 downto 0);
			----------------------------------------------------------
			Ex : out std_logic_Vector(4 downto 0);
			P : out std_logic_vector(63 downto 0)
					
		);			
end FPUSIGNJ;

architecture Behavioral of FPUSIGNJ is

begin

	process (A,B,RM,Funct)
	
	variable varP : std_logic_vector(63 downto 0);
	variable varEx : std_logic_vector(4 downto 0);
	
	begin
	
		varP := (others => '0');
		varEx := (others => '0');
	
		if Funct = "0010000" then

			varP(63 downto 32) := x"FFFFFFFF";

			if and_reduce(A(63 downto 32)) = '0' then 
				varP := x"FFFFFFFF" & x"7FC00000";
			else
				varP := x"FFFFFFFF" & A(31 downto 0);
			end if;

			if and_reduce(B(63 downto 32)) = '1' then

				if RM = "000" then
					varP(31) := B(31);
				elsif RM = "001" then
					varP(31) := not B(31);
				elsif RM = "010" then
					-- varP(31) := A(31) xor B(31);
					varP(31) := varP(31) xor B(31);
				end if;
				
			end if;
			
		elsif Funct = "0010001" then 
		
			varP(62 downto 0) := A(62 downto 0);
			if RM = "000" then
				varP(63) := B(63);
			elsif RM = "001" then
				varP(63) := not B(63);
			elsif RM = "010" then
				varP(63) := A(63) xor B(63);
			end if;
			
		end if;
		
		
		P <= varP;
		EX <= varEX;
		
		
	end process;

end Behavioral;
