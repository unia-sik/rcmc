library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;
use work.libeu.all;


entity FPUMINMAX is
    Port (
				A : in std_logic_vector(63 downto 0);
				B : in std_logic_vector(63 downto 0);
				Funct : in std_logic_vector(6 downto 0);
				RM : in std_logic_vector(2 downto 0);
				----------------------------------------------------------
				Ex : out std_logic_Vector(4 downto 0);
				P : out std_logic_vector(63 downto 0)

		);
end FPUMINMAX;

architecture Behavioral of FPUMINMAX is

begin

	process (A,B,RM,Funct)

	variable varP : std_logic_vector(63 downto 0);
	variable varEx : std_logic_vector(4 downto 0);

	variable nan_box_a : std_logic;
	variable nan_box_b : std_logic;

	begin

		varP := (others => '0');
		varEx := (others => '0');

		nan_box_a := '0';
		nan_box_b := '0';

		if Funct = "0010100" then

			nan_box_a := and_reduce(A(63 downto 32));
			nan_box_b := and_reduce(B(63 downto 32));

			if RM = "000" then
				if nan_box_a = '0' and nan_box_b = '0' then
					-- varEx(4) := '1';
					varP := x"FFFFFFFF" & x"7fc00000";
				elsif nan_box_a = '0' then
					-- varEx(4) := '1';
					varP := x"FFFFFFFF" & B(31 downto 0);
				elsif nan_box_b = '0' then
					-- varEx(4) := '1';
					varP := x"FFFFFFFF" & A(31 downto 0);
				elsif A(30 downto 22) = "111111111" and B(30 downto 22) = "111111111" then
					-- varEx(4) := '1';
					varP := x"FFFFFFFF" & x"7fc00000";
				elsif A(30 downto 22) = "111111110" and unsigned(A(21 downto 0)) /= 0 then
					varEx(4) := '1';
					varP := x"FFFFFFFF" & B(31 downto 0);
				elsif B(30 downto 22) = "111111110" and unsigned(B(21 downto 0)) /= 0 then
					varEx(4) := '1';
					varP := x"FFFFFFFF" & A(31 downto 0);
				elsif A(30 downto 22) = "111111111" then
					varP := x"FFFFFFFF" & B(31 downto 0);
				elsif B(30 downto 22) = "111111111" then
					varP := x"FFFFFFFF" & A(31 downto 0);
				elsif (A(31) xor B(31)) = '1' then
					if A(31) = '1' then
						varP := x"FFFFFFFF" & A(31 downto 0);
					else
						varP := x"FFFFFFFF" & B(31 downto 0);
					end if;
				else
					if A(31) = '1' then
						if unsigned(A(30 downto 0)) > unsigned(B(30 downto 0)) then
							varP := x"FFFFFFFF" & A(31 downto 0);
						else
							varP := x"FFFFFFFF" & B(31 downto 0);
						end if;
					else
						if unsigned(A(30 downto 0)) <= unsigned(B(30 downto 0)) then
							varP := x"FFFFFFFF" & A(31 downto 0);
						else
							varP := x"FFFFFFFF" & B(31 downto 0);
						end if;
					end if;
				end if;
			elsif RM = "001" then
				if nan_box_a = '0' and nan_box_b = '0' then
					-- varEx(4) := '1';
					varP := x"FFFFFFFF" & x"7fc00000";
				elsif nan_box_a = '0' then
					-- varEx(4) := '1';
					varP := x"FFFFFFFF" & B(31 downto 0);
				elsif nan_box_b = '0' then
					-- varEx(4) := '1';
					varP := x"FFFFFFFF" & A(31 downto 0);
				elsif A(30 downto 22) = "111111111" and B(30 downto 22) = "111111111" then
					-- varEx(4) := '1';
					varP := x"FFFFFFFF" & x"7fc00000";
				elsif A(30 downto 22) = "111111110" and unsigned(A(21 downto 0)) /= 0 then
					varEx(4) := '1';
					varP := x"FFFFFFFF" & B(31 downto 0);
				elsif B(30 downto 22) = "111111110" and unsigned(B(21 downto 0)) /= 0 then
					varEx(4) := '1';
					varP := x"FFFFFFFF" & A(31 downto 0);
				elsif A(30 downto 22) = "111111111" then
					varP := x"FFFFFFFF" & B(31 downto 0);
				elsif B(30 downto 22) = "111111111" then
					varP := x"FFFFFFFF" & A(31 downto 0);
				elsif (A(31) xor B(31)) = '1' then
					if A(31) = '1' then
						varP := x"FFFFFFFF" & B(31 downto 0);
					else
						varP := x"FFFFFFFF" & A(31 downto 0);
					end if;
				else
					if A(31) = '1' then
						if unsigned(A(30 downto 0)) > unsigned(B(30 downto 0)) then
							varP := x"FFFFFFFF" & B(31 downto 0);
						else
							varP := x"FFFFFFFF" & A(31 downto 0);
						end if;
					else
						if unsigned(A(30 downto 0)) <= unsigned(B(30 downto 0)) then
							varP := x"FFFFFFFF" & B(31 downto 0);
						else
							varP := x"FFFFFFFF" & A(31 downto 0);
						end if;
					end if;
				end if;
			end if;

		elsif Funct = "0010101" then

			if RM = "000" then
				if A(62 downto 51) = "111111111111" and B(62 downto 51) = "111111111111" then
					-- varEx(4) := '1';
					varP := x"7FF8000000000000";
				elsif A(62 downto 51) = "111111111110" and unsigned(A(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP := B;
				elsif B(62 downto 51) = "111111111110" and unsigned(B(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP := A;
				elsif A(62 downto 51) = "111111111111" then
					varP := B;
				elsif B(62 downto 51) = "111111111111" then
					varP := A;
				elsif (A(63) xor B(63)) = '1' then
					if A(63) = '1' then
						varP := A;
					else
						varP := B;
					end if;
				else
					if A(63) = '1' then
						if unsigned(A(62 downto 0)) > unsigned(B(62 downto 0)) then
							varP := A;
						else
							varP := B;
						end if;
					else
						if unsigned(A(62 downto 0)) <= unsigned(B(62 downto 0)) then
							varP := A;
						else
							varP := B;
						end if;
					end if;
				end if;
			elsif RM = "001" then
				if A(62 downto 51) = "111111111111" and B(62 downto 51) = "111111111111" then
					-- varEx(4) := '1';
					varP := x"7FF8000000000000";
				elsif A(62 downto 51) = "111111111110" and unsigned(A(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP := B;
				elsif B(62 downto 51) = "111111111110" and unsigned(B(50 downto 0)) /= 0 then
					varEx(4) := '1';
					varP := A;
				elsif A(62 downto 51) = "111111111111" then
					varP := B;
				elsif B(62 downto 51) = "111111111111" then
					varP := A;
				elsif (A(63) xor B(63)) = '1' then
					if A(63) = '1' then
						varP := B;
					else
						varP := A;
					end if;
				else
					if A(63) = '1' then
						if unsigned(A(62 downto 0)) > unsigned(B(62 downto 0)) then
							varP := B;
						else
							varP := A;
						end if;
					else
						if unsigned(A(62 downto 0)) <= unsigned(B(62 downto 0)) then
							varP := B;
						else
							varP := A;
						end if;
					end if;
				end if;
			end if;

		end if;


		P <= varP;
		EX <= varEX;


	end process;

end Behavioral;
