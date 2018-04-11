library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;


entity FPUCOMP is
    Port ( 
        A : in std_logic_vector(63 downto 0);
        B : in std_logic_vector(63 downto 0);
        Funct : in std_logic_vector(6 downto 0);
        RM : in std_logic_vector(2 downto 0);
        ----------------------------------------------------------
        Ex : out std_logic_Vector(4 downto 0);
        P : out std_logic_vector(63 downto 0)
    );
end FPUCOMP;

architecture Behavioral of FPUCOMP is

begin

	process (A,B,RM,Funct)
	
	variable varP : std_logic_vector(63 downto 0);
	variable varEx : std_logic_vector(4 downto 0);
	
	begin
	
		varP := (others => '0');
		varEx := (others => '0');
	
		if Funct = "1010000" then

			if and_reduce(A(63 downto 32)) = '1' and and_reduce(B(63 downto 32)) = '1' then 

				if RM = "010" then
					if A(30 downto 22) = "111111110" and unsigned(A(21 downto 0)) /= 0 then
						varEx(4) := '1';
						varP(0) := '0';
					elsif B(30 downto 22) = "111111110" and unsigned(B(21 downto 0)) /= 0 then
						varEx(4) := '1';
						varP(0) := '0';
					elsif A(30 downto 22) = "111111111" or B(30 downto 22) = "111111111" then
						-- varEx(4) := '1';
						varP(0) := '0';
					elsif unsigned(A(30 downto 0)) = 0 and unsigned(B(30 downto 0)) = 0 then
						varP(0) := '1';
					elsif A(31 downto 0) = B(31 downto 0) then
						varP(0) := '1';
					else
						varP(0) := '0';
					end if;
				elsif RM = "001" then
					if A(30 downto 22) = "111111110" and unsigned(A(21 downto 0)) /= 0 then
						varEx(4) := '1';
						varP(0) := '0';
					elsif B(30 downto 22) = "111111110" and unsigned(B(21 downto 0)) /= 0 then
						varEx(4) := '1';
						varP(0) := '0';
					elsif A(30 downto 22) = "111111111" or B(30 downto 22) = "111111111" then
						varEx(4) := '1';
						varP(0) := '0';
					elsif unsigned(A(30 downto 0)) = 0 and unsigned(B(30 downto 0)) = 0 then
						varP(0) := '0';
					elsif (A(31) xor B(31)) = '1' then
						if A(31) = '1' then
							varP(0) := '1';
						else
							varP(0) := '0';
						end if;
					else
						if A(31) = '1' then
							if unsigned(A(30 downto 0)) > unsigned(B(30 downto 0)) then
								varP(0) := '1';
							else
								varP(0) := '0';
							end if;
						else
							if unsigned(A(30 downto 0)) < unsigned(B(30 downto 0)) then
								varP(0) := '1';
							else
								varP(0) := '0';
							end if;
						end if;
					end if;
				elsif RM = "000" then
					if A(30 downto 22) = "111111110" and unsigned(A(21 downto 0)) /= 0 then
						varEx(4) := '1';
						varP(0) := '0';
					elsif B(30 downto 22) = "111111110" and unsigned(B(21 downto 0)) /= 0 then
						varEx(4) := '1';
						varP(0) := '0';
					elsif A(30 downto 22) = "111111111" or B(30 downto 22) = "111111111" then
						varEx(4) := '1';
						varP(0) := '0';
					elsif unsigned(A(30 downto 0)) = 0 and unsigned(B(30 downto 0)) = 0 then
						varP(0) := '1';
					elsif (A(31) xor B(31)) = '1' then
						if A(31) = '1' then
							varP(0) := '1';
						else
							varP(0) := '0';
						end if;
					else
						if A(31) = '1' then
							if unsigned(A(30 downto 0)) >= unsigned(B(30 downto 0)) then
								varP(0) := '1';
							else
								varP(0) := '0';
							end if;
						else
							if unsigned(A(30 downto 0)) <= unsigned(B(30 downto 0)) then
								varP(0) := '1';
							else
								varP(0) := '0';
							end if;
						end if;
					end if;
				end if;
			
			else
				
				varP(0) := '0';

			end if;
			
		elsif Funct = "1010001" then
		
			if RM = "010" then
				if A(62 downto 51) = "111111111110" and unsigned(A(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP(0) := '0';
				elsif B(62 downto 51) = "111111111110" and unsigned(B(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP(0) := '0';
				elsif A(62 downto 51) = "111111111111" or B(62 downto 51) = "111111111111" then
					-- varEx(4) := '1';
					varP(0) := '0';
				elsif unsigned(A(62 downto 0)) = 0 and unsigned(B(62 downto 0)) = 0 then
					varP(0) := '1';
				elsif A = B then
					varP(0) := '1';
				else
					varP(0) := '0';
				end if;
			elsif RM = "001" then
				if A(62 downto 51) = "111111111110" and unsigned(A(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP(0) := '0';
				elsif B(62 downto 51) = "111111111110" and unsigned(B(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP(0) := '0';
				elsif A(62 downto 51) = "111111111111" or B(62 downto 51) = "111111111111" then
					varEx(4) := '1';
					varP(0) := '0';
				elsif unsigned(A(62 downto 0)) = 0 and unsigned(B(62 downto 0)) = 0 then
					varP(0) := '0';
				elsif (A(63) xor B(63)) = '1' then
					if A(63) = '1' then
						varP(0) := '1';
					else
						varP(0) := '0';
					end if;
				else
					if A(63) = '1' then
						if unsigned(A(62 downto 0)) > unsigned(B(62 downto 0)) then
							varP(0) := '1';
						else
							varP(0) := '0';
						end if;
					else
						if unsigned(A(62 downto 0)) < unsigned(B(62 downto 0)) then
							varP(0) := '1';
						else
							varP(0) := '0';
						end if;
					end if;
				end if;
			elsif RM = "000" then
				if A(62 downto 51) = "111111111110" and unsigned(A(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP(0) := '0';
				elsif B(62 downto 51) = "111111111110" and unsigned(B(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP(0) := '0';
				elsif A(62 downto 51) = "111111111111" or B(62 downto 51) = "111111111111" then
					varEx(4) := '1';
					varP(0) := '0';
				elsif unsigned(A(62 downto 0)) = 0 and unsigned(B(62 downto 0)) = 0 then
					varP(0) := '1';
				elsif (A(63) xor B(63)) = '1' then
					if A(63) = '1' then
						varP(0) := '1';
					else
						varP(0) := '0';
					end if;
				else
					if A(63) = '1' then
						if unsigned(A(62 downto 0)) >= unsigned(B(62 downto 0)) then
							varP(0) := '1';
						else
							varP(0) := '0';
						end if;
					else
						if unsigned(A(62 downto 0)) <= unsigned(B(62 downto 0)) then
							varP(0) := '1';
						else
							varP(0) := '0';
						end if;
					end if;
				end if;
			end if;
			
		end if;
		
		
		P <= varP;
		EX <= varEX;
		
		
	end process;

end Behavioral;
