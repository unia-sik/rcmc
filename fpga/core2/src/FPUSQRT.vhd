library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;
use work.libeu.all;


entity FPUSQRT is
    Port ( 
        A : in std_logic_vector(63 downto 0);
        FMT : in std_logic_vector(1 downto 0);
        Funct : in std_logic_vector(6 downto 0);
        RM : in std_logic_vector(2 downto 0);
        ----------------------------------------------------------
        Ex : out std_logic_Vector(4 downto 0);
        P : out std_logic_vector(63 downto 0);
        PA : out std_logic_vector(63 downto 0);
        I : out std_logic_vector(63 downto 0)
    );
end FPUSQRT;

architecture Behavioral of FPUSQRT is

begin

	process (A,FMT,Funct,RM)
	
	variable VarSign : std_logic;
	variable VarExponent : std_logic_vector(10 downto 0);
	variable VarMantisse : std_logic_vector(51 downto 0);
	
	
	variable MaxExponent : natural range 0 to 4095;
	variable DoubleWord : std_logic;
	
	variable VarNaN : std_logic;
	variable VarZero : std_logic;
	variable VarEx : std_logic_vector(4 downto 0);
	
	variable VarP : std_logic_vector(63 downto 0);
	variable VarI : std_logic_vector(63 downto 0);
	
	variable VarPA : std_logic_vector(63 downto 0);
	
	variable Magic : std_logic_vector(63 downto 0);
	
	begin
	
		VarSign := '0';
		VarExponent := (others => '0');
		VarMantisse := (others => '0');
		
		MaxExponent := 0;
		DoubleWord := '0';
		
		VarNaN := '0';
		VarZero := '0';
		VarEx := (others => '0');
		
		VarP := (others => '0');
		VarPA := (others => '0');
		VarI := (others => '0');
		
		Magic := (others => '0');
	
		if Funct = "0101100" and FMT = "00" then
			
			VarSign := A(31);
			VarExponent(7 downto 0) := A(30 downto 23);
			VarMantisse(22 downto 0) := A(22 downto 0);
			MaxExponent := 255;
			Magic := x"FFFFFFFF" & x"5F375A86";
			VarPA := x"FFFFFFFF" & A(31 downto 0);

			if and_reduce(A(63 downto 32)) = '0' then
				VarNaN := '1';
			end if;
			
		elsif Funct = "0101101" and FMT = "01" then
			
			VarSign := A(63);
			VarExponent(10 downto 0) := A(62 downto 52);
			VarMantisse(51 downto 0) := A(51 downto 0);
			MaxExponent := 2047;
			DoubleWord := '1';
			Magic := x"5fE6EB50C7B537A9";
			VarPA := A;
			
		end if;
		
		if VarSign = '1' then
			if unsigned(VarExponent) /= 0 or unsigned(VarMantisse) /= 0 then
				VarNaN := '1';
			else
				VarZero := '1';
			end if;
			VarEx(4) := '1';
		elsif unsigned(VarExponent) = MaxExponent then
			VarNaN := '1';
			VarEx(4) := '1';
		end if;
		
		if VarNaN = '1' then
			
			if DoubleWord = '1' then
				VarP := VarSign & x"FF" & "111" & x"8000000000000";
			else
				VarP := x"FFFFFFFF" & VarSign & x"FF" & "100" & x"00000";
			end if;
			
		elsif VarZero = '1' then
			
			if DoubleWord = '1' then
				VarP := VarSign & x"00" & "000" & x"0000000000000";
			else
				VarP := x"FFFFFFFF" & VarSign & x"00" & "000" & x"00000";
			end if;
		
		else
			
			if DoubleWord = '1' then
				VarP := VarSign & VarExponent & VarMantisse;
			else
				VarP := x"FFFFFFFF" & VarSign & VarExponent(7 downto 0) & VarMantisse(22 downto 0);
			end if;
			
			VarI := std_logic_vector(shift_right(unsigned(VarP),1));
			VarI := std_logic_vector(unsigned(Magic)-unsigned(VarI));
			
		end if;
		
		P <= VarP;
		PA <= VarPA;
		I <= VarI;
		Ex <= VarEx;
		
	end process;

end Behavioral;
