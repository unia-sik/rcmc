library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;
use work.libeu.all;


entity FPUtoEFPU is
    Port ( 
					FPUI : in std_logic_vector(63 downto 0);
					FMT : in std_logic_vector(1 downto 0);
					----------------------------------------------------------
					FPUO : out std_logic_vector(64 downto 0)
					
		);			
end FPUtoEFPU;

architecture Behavioral of FPUtoEFPU is


signal sigSignificand : std_logic_vector(52 downto 0);
signal sigCounter : std_logic_vector(5 downto 0);

begin

	process (FPUI,FMT)
	
	
	begin
		
		if FMT = "00" then
			sigSignificand <= '0' & FPUI(22 downto 0) & "1" & x"1111111";
		elsif FMT = "01" then
			sigSignificand <= '0' & FPUI(51 downto 0);
		else
			sigSignificand <= (others => '1'); 
		end if;
	
	end process;
	
	LZCounter : entity work.LZCounter53Bit(Behavioral) 
		port map (
			A => sigSignificand,
			Z => sigCounter
		);


	process (FPUI,FMT,sigCounter)


	variable VarSign : std_logic;
	variable VarExponent : std_logic_vector(11 downto 0);
	variable VarSignificand : std_logic_vector(51 downto 0);
	
	variable VarCounter : natural range 0 to 63;
	
	
	begin
		
		VarSign := '0';
		VarExponent := (others => '0');
		VarSignificand := (others => '0');
		
		VarCounter := 0;
			
		if FMT = "00" then

			if and_reduce(FPUI(63 downto 32)) = '1' then
			
				VarSign := FPUI(31);
				
				if unsigned(FPUI(30 downto 23)) = 255 then
					VarExponent := (others => '1');
					VarSignificand(51 downto 29) := FPUI(22 downto 0);
				elsif  unsigned(FPUI(30 downto 23)) > 0 then 
					VarExponent := std_logic_vector(resize(unsigned(FPUI(30 downto 23)),12) + 1920);
					VarSignificand(51 downto 29) := FPUI(22 downto 0);	
				elsif unsigned(sigCounter) < 24 then
					VarCounter := to_integer(unsigned(sigCounter));
					VarExponent := std_logic_vector(to_unsigned(1921-VarCounter,12));
					VarSignificand(51 downto 29) := std_logic_vector(shift_left(unsigned(FPUI(22 downto 0)),VarCounter));	
				end if;
				
				FPUO <= VarSign & VarExponent & VarSignificand;
			
			else

				FPUO <= (63 downto 51 => '1', others => '0');

			end if;
		
		elsif FMT = "01" then 
			
			VarSign := FPUI(63);
			
			if unsigned(FPUI(62 downto 52)) = 2047 then
				VarExponent := (others => '1');
				VarSignificand(51 downto 0) := FPUI(51 downto 0);
			elsif  unsigned(FPUI(62 downto 52)) > 0 then 
				VarExponent := std_logic_vector(resize(unsigned(FPUI(62 downto 52)),12) + 1024);
				VarSignificand(51 downto 0) := FPUI(51 downto 0);	
			elsif unsigned(sigCounter) < 53 then
				VarCounter := to_integer(unsigned(sigCounter));
				VarExponent := std_logic_vector(to_unsigned(1025-VarCounter,12));
				VarSignificand(51 downto 0) := std_logic_vector(shift_left(unsigned(FPUI(51 downto 0)),VarCounter));	
			end if;
			
			FPUO <= VarSign & VarExponent & VarSignificand;
		
		end if;
		
		
	end process;

end Behavioral;
