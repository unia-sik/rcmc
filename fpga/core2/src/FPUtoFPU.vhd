library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;
use work.libeu.all;


entity FPUtoFPU is
    Port (
					FPUI : in std_logic_vector(63 downto 0);
					FMT : in std_logic_vector(1 downto 0);
					Funct : in std_logic_vector(6 downto 0);
					RM : in std_logic_vector(2 downto 0);
					----------------------------------------------------------
					Ex : out std_logic_Vector(4 downto 0);
					FPUO : out std_logic_vector(63 downto 0)

		);
end FPUtoFPU;

architecture Behavioral of FPUtoFPU is


signal sigSignificand : std_logic_vector(31 downto 0);
signal sigCounter : std_logic_vector(4 downto 0);

begin

	process (FPUI,FMT,Funct)


	begin

		if Funct(0) = '1' and FMT(0) = '0' then
			sigSignificand <= '0' & FPUI(22 downto 0) & x"FF";
		else
			sigSignificand <= (others => '1');
		end if;

	end process;

	LZCounter : entity work.LZCounter32Bit(Behavioral)
		port map (
			A => sigSignificand,
			Z => sigCounter
		);


	process (FPUI,RM,Funct,FMT,sigCounter)

	variable VarEX : std_logic_vector(4 downto 0);
	variable VarSign : std_logic;
	variable VarExponent : integer range -2048 to 2047;
	variable VarSignificand : std_logic_vector(78 downto 0);
	variable VarSignificand_Round : std_logic_vector(24 downto 0);

	variable VarCounter : natural range 0 to 31;
	variable VarSubnormal : std_logic;

	variable G : std_logic;
	variable R : std_logic;
	variable S : std_logic;
	variable GRS : std_logic;
	variable Even : std_logic;

	variable Round : boolean;


	begin

		VarEX := (others => '0');
		VarSign := '0';
		VarExponent := 0;
		VarSignificand := (others => '0');
		VarSignificand_Round := (others => '0');
		VarSubnormal := '0';
		GRS := '0';

		VarCounter := 0;

		Round := false;

		if Funct(0) = '0' and FMT(0) = '1' then

			VarSign := FPUI(63);

			if  unsigned(FPUI(62 downto 52)) = 2047 then
				if unsigned(FPUI(51 downto 0)) /= 0 then
					VarSign := '0';
					VarEX(4) := not FPUI(51);
					VarSignificand(77) := '1';
				end if;
				VarExponent := 255;
			elsif unsigned(FPUI(62 downto 52)) > 1150 then
				VarEX(2) := '1';
				VarEX(0) := '1';
				VarExponent := 255;
			elsif unsigned(FPUI(62 downto 52)) > 896 then
				VarExponent := to_integer(unsigned(FPUI(62 downto 52))) - 896;
				VarSignificand(78 downto 26) := '1' & FPUI(51 downto 0);
			elsif unsigned(FPUI(62 downto 52)) > 871 then
				VarExponent := 0;
				VarSignificand(78 downto 26) := '1' & FPUI(51 downto 0);
				VarCounter := to_integer(897 - unsigned(FPUI(62 downto 52)));
				VarSignificand := std_logic_vector(shift_right(unsigned(VarSignificand),VarCounter));
				VarSubnormal := '1';
			elsif unsigned(FPUI(61 downto 0)) /= 0 then
				VarExponent := -1;
				VarSignificand(78 downto 26) := '1' & FPUI(51 downto 0);
				VarEX(1) := '1';
				VarEX(0) := '1';
			end if;

			VarSignificand_Round(23 downto 0) := VarSignificand(78 downto 55);

			G := VarSignificand(54);
			R := VarSignificand(53);
			S := or_reduce(VarSignificand(52 downto 0));

			GRS := G or R or S;

			Even := VarSignificand_Round(0) or R or S;
	
			if GRS = '1' then
				VarEX(0) := '1';
			end if;

			if VarSignificand(77 downto 54) = x"FFFFFF" and R = '0' and VarExponent = 0 then
				VarEX(1) := GRS;
			end if;

			case RM is

				when "000" => --RNE--

					if G='1' and Even = '1' then
						Round := true;
					end if;

				when "001" => --RTZ--

				when "010" => --RDN--

					if VarSign = '1' and GRS = '1' then
						Round := true;
					end if;

				when "011" => --RUP--

					if VarSign = '0' and GRS = '1' then
						Round := true;
					end if;

				when "100" => --RMM--

					if GRS = '1' then
						Round := true;
					end if;

				when others =>

			end case;

			if Round then
				VarSignificand_Round := std_logic_vector(unsigned(VarSignificand_Round) + 1);
			end if;

			if VarSubnormal = '0' then
				if VarSignificand_Round(24) = '1' then
					VarExponent := VarExponent+1;
					VarSignificand_Round := std_logic_vector(shift_right(unsigned(VarSignificand_Round),1));
				end if;
			else
				if VarSignificand_Round(23) = '1' then
					VarExponent := VarExponent+1;
				end if;
			end if;

			if VarSignificand_Round(23) = '0' and VarExponent = 0 then
				VarEX(1) := GRS;
			end if;

			if Round and VarExponent = 255 then
				VarEX(2) := '1';
				VarEX(0) := '1';
			end if;

			if VarExponent < 0 then
				FPUO <= x"FFFFFFFF" & VarSign & x"00" & "000" & x"00000";
			else
				FPUO <= x"FFFFFFFF" & VarSign & std_logic_vector(to_unsigned(VarExponent,8)) & VarSignificand_Round(22 downto 0);
			end if;

		elsif Funct(0) = '1' and FMT(0) = '0' then


			if and_reduce(FPUI(63 downto 32)) = '1' then

				VarSign := FPUI(31);

				if unsigned(FPUI(30 downto 23)) = 255 then
					VarExponent := 2047;
					if unsigned(FPUI(22 downto 0)) /= 0 then
						VarSignificand(51) := '1';
						VarEX(4) := not FPUI(22);
						VarSign := '0';
					end if;
				elsif  unsigned(FPUI(30 downto 23)) > 0 then
					VarExponent := to_integer(unsigned(FPUI(30 downto 23))) + 896;
					VarSignificand(51 downto 29) := FPUI(22 downto 0);
				elsif unsigned(sigCounter) < 24 then
					VarCounter := to_integer(unsigned(sigCounter));
					VarExponent := 897-VarCounter;
					VarSignificand(51 downto 29) := std_logic_vector(shift_left(unsigned(FPUI(22 downto 0)),VarCounter));
				end if;

				FPUO <= VarSign & std_logic_vector(to_unsigned(VarExponent,11)) & VarSignificand(51 downto 0);
			
			else

				FPUO <= (62 downto 51 => '1', others => '0');
				
			end if;

		else

			FPUO <= (others => '0');

		end if;


		EX <= varEX;


	end process;

end Behavioral;
