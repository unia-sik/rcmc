library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;


entity FPUCLASS is
    Port ( 
        A : in std_logic_vector(63 downto 0);
        D : in std_logic_vector(63 downto 0);
        FMT : in std_logic_vector(1 downto 0);
        Funct : in std_logic_vector(6 downto 0);
        RM : in std_logic_vector(2 downto 0);
        AuxI : in std_logic;
        ----------------------------------------------------------
        Ex : out std_logic_Vector(4 downto 0);
        P : out std_logic_vector(63 downto 0)
    );
end FPUCLASS;

architecture Behavioral of FPUCLASS is

begin

	process (A,D,RM,Funct,FMT,AuxI)
	
	variable varP : std_logic_vector(63 downto 0);
	variable varEx : std_logic_vector(4 downto 0);
	
	begin
	
		varP := (others => '0');
		varEx := (others => '0');
	
		if Funct = "1110000" then
			
			if AuxI = '0' and RM = "000" and FMT = "00" then
			
				varP := std_logic_vector(resize(signed(A(31 downto 0)),64));
		
			elsif AuxI = '1' and RM = "001" and FMT = "00" then
			
				if A(31) = '1' then
					if A(30 downto 23) = "11111111" then
						if unsigned(A(22 downto 0)) = 0 then
							varP(0) := '1';
						elsif A(22) = '0' then
							varP(8) := '1';
						else
							varP(9) := '1';
						end if;
					elsif A(30 downto 23) = "00000000" then
						if unsigned(A(22 downto 0)) = 0 then
							varP(3) := '1';
						else
							varP(2) := '1';
						end if;
					else
						varP(1) := '1';
					end if;
				else		
					if A(30 downto 23) = "11111111" then
						if unsigned(A(22 downto 0)) = 0 then
							varP(7) := '1';
						elsif A(22) = '0' then
							varP(8) := '1';
						else
							varP(9) := '1';
						end if;
					elsif A(30 downto 23) = "00000000" then
						if unsigned(A(22 downto 0)) = 0 then
							varP(4) := '1';
						else
							varP(5) := '1';
						end if;
					else
						varP(6) := '1';
					end if;
				end if;
				
			end if;
			
		elsif Funct = "1110001" then
		
			if AuxI = '0' and RM = "000" and FMT = "00" then
				
				varP := A;
		
			elsif AuxI = '1' and RM = "001" and FMT = "00" then
			
				if A(63) = '1' then
					if A(62 downto 52) = "11111111111" then
						if unsigned(A(51 downto 0)) = 0 then
							varP(0) := '1';
						elsif A(51) = '0' then
							varP(8) := '1';
						else
							varP(9) := '1';
						end if;
					elsif A(62 downto 52) = "00000000000" then
						if unsigned(A(51 downto 0)) = 0 then
							varP(3) := '1';
						else
							varP(2) := '1';
						end if;
					else
						varP(1) := '1';
					end if;
				else		
					if A(62 downto 52) = "11111111111" then
						if unsigned(A(51 downto 0)) = 0 then
							varP(7) := '1';
						elsif A(51) = '0' then
							varP(8) := '1';
						else
							varP(9) := '1';
						end if;
					elsif A(62 downto 52) = "00000000000" then
						if unsigned(A(51 downto 0)) = 0 then
							varP(4) := '1';
						else
							varP(5) := '1';
						end if;
					else
						varP(6) := '1';
					end if;
				end if;
				
			end if;
		
		elsif Funct = "1111000" then
		
			if RM = "000" and FMT = "00" then
			
				varP := x"FFFFFFFF" & D(31 downto 0);
			
			end if;
		
		elsif Funct = "1111001" then
			
			if RM = "000" and FMT = "00" then
			
				varP := D;
				
			end if;
			
		end if;
		
		
		P <= varP;
		EX <= varEX;
		
		
	end process;

end Behavioral;
