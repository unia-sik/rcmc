library IEEE;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;


entity FPUDIV is
    Port ( 
        A : in std_logic_vector(63 downto 0);
        B : in std_logic_vector(63 downto 0);
        FMT : in std_logic_vector(1 downto 0);
        Funct : in std_logic_vector(6 downto 0);
        RM : in std_logic_vector(2 downto 0);
        ----------------------------------------------------------
        Ex : out std_logic_Vector(4 downto 0);
        P : out std_logic_vector(63 downto 0);
        PA : out std_logic_vector(63 downto 0);
        PB : out std_logic_vector(63 downto 0)
    );
end FPUDIV;

architecture Behavioral of FPUDIV is

begin

	process (A,B,FMT,Funct,RM)
	
	
	variable VarSign_A : std_logic;
	variable VarExponent_A : std_logic_vector(10 downto 0);
	variable VarMantisse_A : std_logic_vector(51 downto 0);
	
	variable VarSign_B : std_logic;
	variable VarExponent_B : std_logic_vector(10 downto 0);
	variable VarMantisse_B : std_logic_vector(51 downto 0);
	
	variable VarNaN : std_logic;
	variable VarInf : std_logic;
	variable VarDivZero : std_logic;
	variable VarDivInf : std_logic;
	variable DoubleWord : std_logic;
	
	variable VarEx : std_logic_vector(4 downto 0);
	
	variable MaxExponent : integer range 0 to 4095;
	variable BiasExponent : integer range 0 to 2047;
	variable DiffExponent : integer range -2047 to 2047;
	
	variable VarP : std_logic_vector(63 downto 0);
	variable VarA : std_logic_vector(63 downto 0);
	variable VarB : std_logic_vector(63 downto 0);
	
	
	variable VarSign_P : std_logic;
	
	
	begin
	
		VarSign_A := '0';
		VarExponent_A := (others => '0');
		VarMantisse_A := (others => '0');
		
		VarSign_B := '0';
		VarExponent_B := (others => '0');
		VarMantisse_B := (others => '0');
		
		MaxExponent := 0;
		BiasExponent := 0;
		DiffExponent := 0;
		
		VarNaN := '0';
		VarInf := '0';
		VarDivZero := '0';
		VarDivInf := '0';
		DoubleWord := '0';
		
		VarP := (others => '0');
		VarA := (others => '0');
		VarB := (others => '0');
		
		VarEx := (others => '0');
		
		VarSign_P := '0';
	
		if Funct = "0001100" and FMT = "00"then
			
			VarSign_A := A(31);
			VarExponent_A(7 downto 0) := A(30 downto 23);
			VarMantisse_A(22 downto 0) := A(22 downto 0);
			
			VarSign_B := B(31);
			VarExponent_B(7 downto 0) := B(30 downto 23);
			VarMantisse_B(22 downto 0) := B(22 downto 0);
			
			VarSign_P := A(31) xor B(31);
			
			MaxExponent := 255;
			BiasExponent := 126;
			DoubleWord := '0';

			if and_reduce(A(63 downto 32)) = '0' or and_reduce(B(63 downto 32)) = '0' then
				VarNaN := '1';
			end if;
			
		elsif Funct = "0001101" and FMT = "01" then
			
			VarSign_A := A(63);
			VarExponent_A := A(62 downto 52);
			VarMantisse_A := A(51 downto 0);
			
			VarSign_B := B(63);
			VarExponent_B := B(62 downto 52);
			VarMantisse_B := B(51 downto 0);
			
			VarSign_P := A(63) xor B(63);
			
			MaxExponent := 2047;
			BiasExponent := 1022;
			DoubleWord := '1';
			
		end if;
		
		
		if unsigned(VarExponent_A) = MaxExponent then
			if unsigned(VarMantisse_A) /= 0 then
				VarNaN := '1';
			else
				VarInf := '1';
			end if;
		end if;		
		
		if unsigned(VarExponent_B) = MaxExponent then
			if unsigned(VarMantisse_B) /= 0 then
				VarNaN := '1';
			else
				if VarInf = '1' then
					VarNaN := '1';
				else
					VarDivInf := '1';
				end if;
			end if;
		elsif unsigned(VarExponent_B) = 0 then
			if unsigned(VarMantisse_B) = 0 then
				VarDivZero := '1';
			end if;
		end if;
		
		if VarNaN = '1' then
			
			VarEx(4) := '1';
			
			if DoubleWord = '0' then
				VarP := x"FFFFFFFF"  & VarSign_P & x"FF" & "100" & x"00000";
			else
				VarP := VarSign_P & x"FF" & "111" & x"1000000000000";
			end if;
		
		elsif VarDivZero = '1' then
			
			VarEx(3) := '1';
			
			if DoubleWord = '0' then
				VarP := x"FFFFFFFF"  & VarSign_P & x"FF" & "000" & x"00000";
			else
				VarP := VarSign_P & x"FF" & "111" & x"0000000000000";
			end if;
		
		elsif VarInf = '1' then
			
			if DoubleWord = '0' then
				VarP := x"FFFFFFFF"  & VarSign_P & x"FF" & "000" & x"00000";
			else
				VarP := VarSign_P & x"FF" & "111" & x"0000000000000";
			end if;
			
		elsif VarDivInf = '1' then
			
			if DoubleWord = '0' then
				VarP := x"FFFFFFFF"  & VarSign_P & "000" & x"0000000";
			else
				VarP := VarSign_P & "000" & "000000000000000";
			end if;
		
		else
			
			DiffExponent := to_integer(unsigned(VarExponent_B)) - BiasExponent;
			
			VarExponent_B := std_logic_vector(to_unsigned(BiasExponent,11));
			
			VarExponent_A := std_logic_vector(to_unsigned((to_integer(unsigned(VarExponent_A))-DiffExponent),11));
			
			if DoubleWord = '0' then
				VarA := x"FFFFFFFF"  & VarSign_A & VarExponent_A(7 downto 0) & VarMantisse_A(22 downto 0);
				VarB := x"FFFFFFFF"  & VarSign_B & VarExponent_B(7 downto 0) & VarMantisse_B(22 downto 0);
			else
				VarA := VarSign_A & VarExponent_A & VarMantisse_A;
				VarB := VarSign_B & VarExponent_B & VarMantisse_B;
			end if;
			
		end if;
		
		Ex <= VarEx;
		P <= VarP;
		PA <= VarA;
		PB <= VarB;
		
	end process;

end Behavioral;
