library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;
use work.constants.all;


entity INTtoFPU is
    Port (
        INT : in std_logic_vector(63 downto 0);
        FMT : in std_logic_vector(1 downto 0);
        Funct : in std_logic_vector(6 downto 0);
        RM : in std_logic_vector(2 downto 0);
        FPUOP : in  std_logic;
        nRST : in std_logic;
        CLK : in  std_logic;
        ----------------------------------------------------------
        Ex : out std_logic_Vector(4 downto 0);
        FPU : out std_logic_vector(63 downto 0);
        Ready : out  std_logic
    );
end INTtoFPU;

architecture Behavioral of INTtoFPU is

signal Sign : std_logic;
signal Counter : std_logic_vector(5 downto 0);
signal Significand : std_logic_vector(129 downto 0);


signal Sign_2 : std_logic;
signal Counter_2 : std_logic_vector(5 downto 0);
signal Significand_2 : std_logic_vector(129 downto 0);

signal Funct_2 : std_logic_vector(6 downto 0);
signal RM_2 : std_logic_vector(2 downto 0);

signal sigReady : std_logic;
signal sigReady_2 : std_logic;

begin
	process (INT,RM,Funct,FMT,FPUOP)

	variable VarSign : std_logic;
	variable VarSignificand : std_logic_vector(129 downto 0);

	begin

		VarSign := '0';
		VarSignificand := (others => '0');

		if FPUOP = '0' then

			sigReady <= '0';

		elsif  Funct(6 downto 1) = "110100" then

			if FMT = "00" then

				VarSign := INT(31);
				if VarSign = '1' then
					VarSignificand(97 downto 66) := std_logic_vector(unsigned(not INT(31 downto 0))+1);
				else
					VarSignificand(96 downto 66) := INT(30 downto 0);
				end if;

			elsif FMT = "01" then

				VarSign := '0';
				VarSignificand(97 downto 66) := INT(31 downto 0);

			elsif FMT = "10" then

				VarSign := INT(63);
				if VarSign = '1' then
					VarSignificand(129 downto 66) := std_logic_vector(unsigned(not INT(63 downto 0))+1);
				else
					VarSignificand(128 downto 66) := INT(62 downto 0);
				end if;

			else

				VarSign := '0';
				VarSignificand(129 downto 66) := INT(63 downto 0);

			end if;

			sigReady <= '1';

		else

			sigReady <= '0';

		end if;

		Sign <= VarSign;
		Significand <= VarSignificand;

	end process;



	LZCounter : entity work.LZCounter64Bit(Behavioral)
		port map (
			A => Significand(129 downto 66),
			Z => Counter
		);


	process(CLK,nRST)


	begin

		if nRST = '0' then
			sigReady_2 <= '0';
			RM_2 <= (others => '0');
			Funct_2 <= (others => '0');
			Sign_2 <= '0';
			Counter_2 <= (others => '0');
			Significand_2 <= (others => '0');

		elsif rising_edge(CLK) then
			sigReady_2 <= sigReady;
			RM_2 <= RM;
			Funct_2 <= Funct;
			Sign_2 <= Sign;
			Counter_2 <= Counter;
			Significand_2 <= Significand;

		end if;

	end process;


	process(RM_2,Funct_2,Counter_2,Sign_2,Significand_2,sigReady_2)

	variable VarSign : std_logic;
	variable VarEX : std_logic_vector(4 downto 0);
	variable VarExponent : std_logic_vector(10 downto 0);
	variable VarSignificand : std_logic_vector(129 downto 0);
	variable VarSignificand_Round : std_logic_vector(53 downto 0);

	variable G : std_logic;
	variable R : std_logic;
	variable S : std_logic;
	variable GRS : std_logic;
	variable Even : std_logic;

	variable VarCounter : natural range 0 to 63;

	variable Round : boolean;

	begin

		VarCounter := 63-to_integer(unsigned(Counter_2));
		VarSign := Sign_2;
		VarEX := (others => '0');
		VarExponent := (others => '0');
		VarSignificand := Significand_2;
		VarSignificand_Round := (others => '0');
		GRS := '0';

		Round := false;

		if unsigned(VarSignificand) /= 0 then

			if Funct_2(0) = '0' then

				VarExponent := std_logic_vector(to_unsigned(127,11)+VarCounter);

			elsif  Funct_2(0) = '1' then

				VarExponent := std_logic_vector(to_unsigned(1023,11)+VarCounter);

			end if;

			VarSignificand := std_logic_vector(shift_right(unsigned(VarSignificand),VarCounter));
			VarSignificand_Round := VarSignificand(67 downto 14);


			if Funct_2(0) = '0' then

				G := VarSignificand(42);
				R := VarSignificand(41);
				S := or_reduce(VarSignificand(40 downto 0));

				GRS := G or R or S;

				Even := VarSignificand_Round(29) or R or S;

        case RM_2 is

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
          VarSignificand_Round(53 downto 29) := std_logic_vector(unsigned(VarSignificand_Round(53 downto 29)) + 1);
        end if;

			else

				G := VarSignificand(13);
				R := VarSignificand(12);
				S := or_reduce(VarSignificand(11 downto 0));

				GRS := G or R or S;

				Even := VarSignificand_Round(0) or R or S;

        case RM_2 is

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

			end if;

			if VarSignificand_Round(53) = '1' then
				VarExponent := std_logic_vector(unsigned(VarExponent)+1);
				VarSignificand_Round := std_logic_vector(shift_right(unsigned(VarSignificand_Round),1));
			end if;

		end if;

		if Funct_2(0) = '0' then
			FPU <= x"ffffffff" & VarSign & VarExponent(7 downto 0) & VarSignificand_Round(51 downto 29);
		else
			FPU <= VarSign & VarExponent & VarSignificand_Round(51 downto 0);
		end if;
    
    if GRS = '1' then
        VarEX(0) := '1';
    end if;

		Ready <= sigReady_2;
		EX <= varEX;

	end process;

end Behavioral;
