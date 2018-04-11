library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;
use work.libeu.all;


entity FPUFMA is
	Port 
	(
		A     : in  std_logic_vector(63 downto 0);
		B     : in  std_logic_vector(63 downto 0);
		C     : in  std_logic_vector(63 downto 0);
		FMT   : in  std_logic_vector(1 downto 0);
		Funct : in  std_logic_vector(6 downto 0);
		RM    : in  std_logic_vector(2 downto 0);
		FPUOP : in  std_logic;
		nRST  : in  std_logic;
		CLK   : in  std_logic;
		----------------------------------------------------------
		Ex    : out std_logic_Vector(4 downto 0);
		P     : out std_logic_vector(63 downto 0);
		Ready : out std_logic
	);
end FPUFMA;





architecture Behavioral of FPUFMA is


signal VarA             : std_logic_vector(63 downto 0);
signal VarB             : std_logic_vector(63 downto 0);
signal VarC             : std_logic_vector(63 downto 0);

signal VarA_2           : std_logic_vector(64 downto 0);
signal VarB_2           : std_logic_vector(64 downto 0);
signal VarC_2           : std_logic_vector(64 downto 0);

signal Sign_A           : std_logic;
signal Exponent_A       : natural range 0 to 4095;
signal Significand_A    : std_logic_vector(52 downto 0);

signal Sign_B           : std_logic;
signal Exponent_B       : natural range 0 to 4095;
signal Significand_B    : std_logic_vector(52 downto 0);

signal Sign_C           : std_logic;
signal Exponent_C       : natural range 0 to 4095;
signal Significand_C    : std_logic_vector(52 downto 0);

signal varFMT           : std_logic_vector(1 downto 0);
signal VarSNaN          : std_logic;
signal VarQNaN          : std_logic;
signal VarInf           : std_logic;
signal VarRM            : std_logic_vector(2 downto 0);
signal varFMAOP         : std_logic_vector(2 downto 0);

signal Sign_Addition        : std_logic;
signal Exponent_Addition    : natural range 0 to 4095;
signal Significand_Addition : std_logic_vector(162 downto 0);

signal Sign_Multiply        : std_logic;
signal Exponent_Multiply    : integer range 0 to 8191;
signal Significand_Multiply : std_logic_vector(162 downto 0);


signal varFMT_2                 : std_logic_vector(1 downto 0);
signal VarSNaN_2                : std_logic;
signal VarQNaN_2                : std_logic;
signal VarInf_2                 : std_logic;
signal VarRM_2                  : std_logic_vector(2 downto 0);
signal varFMAOP_2               : std_logic_vector(2 downto 0);

signal Sign_Addition_2          : std_logic;
signal Exponent_Addition_2      : natural range 0 to 8191;
signal Significand_Addition_2   : std_logic_vector(162 downto 0);

signal Sign_Multiply_2       	: std_logic;
signal Exponent_Multiply_2   	: natural range 0 to 8191;
signal Significand_Multiply_2	: std_logic_vector(162 downto 0);


signal varFMT_3               : std_logic_vector(1 downto 0);
signal VarSNaN_3              : std_logic;
signal VarQNaN_3           	: std_logic;
signal VarInf_3               : std_logic;
signal VarRM_3                : std_logic_vector(2 downto 0);
signal varFMAOP_3             : std_logic_vector(2 downto 0);

signal Sign_Addition_3        : std_logic;
signal Exponent_Addition_3    : natural range 0 to 8191;
signal Significand_Addition_3 : std_logic_vector(162 downto 0);

signal Sign_Multiply_3        : std_logic;
signal Exponent_Multiply_3    : natural range 0 to 8191;
signal Significand_Multiply_3 : std_logic_vector(162 downto 0);



signal Sign_Add          : std_logic;
signal Exponent_Add      : natural range 0 to 8191;
signal Significand_Add   : std_logic_vector(162 downto 0);
signal Counter_Add       : std_logic_vector(7 downto 0);



signal Sign_Add_2        : std_logic;
signal Exponent_Add_2    : natural range 0 to 8191;
signal Significand_Add_2 : std_logic_vector(162 downto 0);
signal Counter_Add_2     : std_logic_vector(7 downto 0);


signal VarSign           : std_logic;
signal VarExponent       : natural range 0 to 8191;
signal VarSignificand    : std_logic_vector(162 downto 0);



signal sigReady   : std_logic;
signal sigReady_2 : std_logic;
signal sigReady_3 : std_logic;

begin
	
	Initial : process (A,B,C,Funct,FMT,FPUOP)
	
	begin
		
		
			
		if FPUOP = '0' then
		
			sigReady <= '0';
			
			VarA <= (others => '0');
			VarB <= (others => '0');
			VarC <= (others => '0');
			VarFMAOP <= (others => '0');
		
		else 
		
			if FMT = "00" then
				
				if Funct = FUNC_FMADD then
					sigReady <= '1';
					
					VarA <= A(63 downto 32) & A(31 downto 0);
					VarB <= B(63 downto 32) & B(31 downto 0);
					VarC <= C(63 downto 32) & C(31 downto 0);
					VarFMAOP <= "000";
				elsif Funct = FUNC_FMSUB then
					sigReady <= '1';
					
					VarA <= A(63 downto 32) & A(31 downto 0);
					VarB <= B(63 downto 32) & B(31 downto 0);
					VarC <= C(63 downto 32) & (not C(31)) & C(30 downto 0);
					VarFMAOP <= "001";
				elsif Funct = FUNC_FNMSUB then
					sigReady <= '1';
					
					VarA <= A(63 downto 32) & A(31 downto 0);
					VarB <= B(63 downto 32) & B(31 downto 0);
					VarC <= C(63 downto 32) & (not C(31)) & C(30 downto 0);
					VarFMAOP <= "010";
				elsif Funct = FUNC_FNMADD then
					sigReady <= '1';
					
					VarA <= A(63 downto 32) & A(31 downto 0);
					VarB <= B(63 downto 32) & B(31 downto 0);
					VarC <= C(63 downto 32) & C(31 downto 0);
					VarFMAOP <= "011";
				elsif Funct = FUNC_FADD_S then
					sigReady <= '1';
					
					VarA <= A(63 downto 32) & A(31 downto 0);
					VarB <= x"FFFFFFFF" & x"3F800000";
					VarC <= C(63 downto 32) & B(31 downto 0);
					VarFMAOP <= "100";
				elsif Funct = FUNC_FSUB_S then
					sigReady <= '1';
					
					VarA <= A(63 downto 32) & A(31 downto 0);
					VarB <= x"FFFFFFFF" & x"3F800000";
					VarC <= C(63 downto 32) & (not B(31)) & B(30 downto 0);
					VarFMAOP <= "101";
				elsif Funct = FUNC_FMUL_S then
					sigReady <= '1';
					
					VarA <= A(63 downto 32) & A(31 downto 0);
					VarB <= B(63 downto 32) & B(31 downto 0);
					VarC <= (63 downto 32 => '1',31 => A(31) xor B(31),others => '0');
					VarFMAOP <= "110";
				else
					sigReady <= '0';
					
					VarA <= (others => '0');
					VarB <= (others => '0');
					VarC <= (others => '0');
					VarFMAOP <= (others => '0');
				end if;
			
			elsif FMT = "01" then
			
				if Funct = FUNC_FMADD then
					sigReady <= '1';
					
					VarA <= A(63 downto 0);
					VarB <= B(63 downto 0);
					VarC <= C(63 downto 0);
					VarFMAOP <= "000";
				elsif Funct = FUNC_FMSUB then
					sigReady <= '1';
					
					VarA <= A(63 downto 0);
					VarB <= B(63 downto 0);
					VarC <= (not C(63)) & C(62 downto 0);
					VarFMAOP <= "001";
				elsif Funct = FUNC_FNMSUB then
					sigReady <= '1';
					
					VarA <= A(63 downto 0);
					VarB <= B(63 downto 0);
					VarC <= (not C(63)) & C(62 downto 0);
					VarFMAOP <= "010";
				elsif Funct = FUNC_FNMADD then
					sigReady <= '1';
					
					VarA <= A(63 downto 0);
					VarB <= B(63 downto 0);
					VarC <= C(63 downto 0); 
					VarFMAOP <= "011";
				elsif Funct = FUNC_FADD_D then
					sigReady <= '1';
					
					VarA <= A(63 downto 0);
					VarB <= x"3FF0000000000000";
					VarC <= B(63 downto 0);
					VarFMAOP <= "100";
				elsif Funct = FUNC_FSUB_D then
					sigReady <= '1';
					
					VarA <= A(63 downto 0);
					VarB <= x"3FF0000000000000";
					VarC <= (not B(63)) & B(62 downto 0);
					VarFMAOP <= "101";
				elsif Funct = FUNC_FMUL_D then
					sigReady <= '1';

					VarA <= A(63 downto 0);
					VarB <= B(63 downto 0);
					VarC <= (63 => A(63) xor B(63),others => '0');
					VarFMAOP <= "110";
				else
					sigReady <= '0';

					VarA <= (others => '0');
					VarB <= (others => '0');
					VarC <= (others => '0');
					VarFMAOP <= (others => '0');
				end if;
				
			end if;
		
		end if;
		
	
	end process;
	
	

	FPUtoEFPU_A : entity work.FPUtoEFPU(Behavioral) 
		port map(
			FPUI => VarA,
			FMT  => FMT,
			FPUO => VarA_2
		);

	FPUtoEFPU_B : entity work.FPUtoEFPU(Behavioral) 
		port map(
			FPUI => VarB,
			FMT  => FMT,
			FPUO => VarB_2
		);

	FPUtoEFPU_C : entity work.FPUtoEFPU(Behavioral) 
		port map(
			FPUI => VarC,
			FMT  => FMT,
			FPUO => VarC_2
		);
	

	InitialValue : process (VarA_2,VarB_2,VarC_2)
	
	begin
		

		Sign_A <= VarA_2(64);
		Significand_A(51 downto 0) <= VarA_2(51 downto 0);
		
		if unsigned(VarA_2(63 downto 52)) = 0 then
			if unsigned(VarA_2(51 downto 0)) /= 0 then
				Exponent_A <= 1;
			else
				Exponent_A <= 0;
			end if;
			Significand_A(52) <= '0';
		elsif unsigned(VarA_2(63 downto 52)) = 4095 then
			Exponent_A <= 4095;
			Significand_A(52) <= '1';
		else
			Exponent_A <= to_integer(unsigned(VarA_2(63 downto 52)));
			Significand_A(52) <= '1';
		end if;

		
		Sign_B <= VarB_2(64);
		Significand_B(51 downto 0) <= VarB_2(51 downto 0);
		
		if unsigned(VarB_2(63 downto 52)) = 0 then
			if unsigned(VarB_2(51 downto 0)) /= 0 then
				Exponent_B <= 1;
			else
				Exponent_B <= 0;
			end if;
			Significand_B(52) <= '0';
		elsif unsigned(VarB_2(63 downto 52)) = 4095 then
			Exponent_B <= 4095;
			Significand_B(52) <= '1';
		else
			Exponent_B <= to_integer(unsigned(VarB_2(63 downto 52)));
			Significand_B(52) <= '1';
		end if;

		
		Sign_C <= VarC_2(64);
		Significand_C(51 downto 0) <= VarC_2(51 downto 0);
		
		if unsigned(VarC_2(63 downto 52)) = 0 then
			if unsigned(VarC_2(51 downto 0)) /= 0 then
				Exponent_C <= 1;
			else
				Exponent_C <= 0;
			end if;
			Significand_C(52) <= '0';
		elsif unsigned(VarC_2(63 downto 52)) = 4095 then
			Exponent_C <= 4095;
			Significand_C(52) <= '1';
		else
			Exponent_C <= to_integer(unsigned(VarC_2(63 downto 52)));
			Significand_C(52) <= '1';
		end if;
	
	end process;
	
	
	
	
	check_NaN : process(VarA_2,VarB_2,VarC_2,RM,FMT)
	
	begin
	
		VarRM <= RM;
		varFMT <= FMT;
		
		if (unsigned(VarA_2(63 downto 52)) = 4095 and VarA_2(51) = '0' and unsigned(VarA_2(50 downto 0)) /= 0)
			or (unsigned(VarB_2(63 downto 52)) = 4095 and VarB_2(51) = '0' and unsigned(VarB_2(50 downto 0)) /= 0) 
			or (unsigned(VarC_2(63 downto 52)) = 4095 and VarC_2(51) = '0' and unsigned(VarC_2(50 downto 0)) /= 0) then
			
			VarSNaN <= '1';
			VarQNaN <= '0';
                    
        elsif (unsigned(VarA_2(63 downto 0)) = 0 and unsigned(VarB_2(63 downto 52)) = 4095 and unsigned(VarB_2(51 downto 0)) = 0)
            or (unsigned(VarB_2(63 downto 0)) = 0 and unsigned(VarA_2(63 downto 52)) = 4095 and unsigned(VarA_2(51 downto 0)) = 0) then
                
            VarSNaN <= '1';
            VarQNaN <= '0';
		
		elsif (unsigned(VarA_2(63 downto 52)) = 4095 and VarA_2(51) = '1') 
			or (unsigned(VarB_2(63 downto 52)) = 4095 and VarB_2(51) = '1')
			or (unsigned(VarC_2(63 downto 52)) = 4095 and VarC_2(51) = '1') then
			
			VarSNaN <= '0';
			VarQNaN <= '1';
		
		elsif (((unsigned(VarA_2(63 downto 52)) = 4095 and unsigned(VarA_2(51 downto 0)) = 0) or (unsigned(VarB_2(63 downto 52)) = 4095 and unsigned(VarB_2(51 downto 0)) = 0))
            and unsigned(VarC_2(63 downto 52)) = 4095 and unsigned(VarC_2(51 downto 0)) = 0 and ((VarA_2(64) xor VarB_2(64)) /= VarC_2(64))) then
                        
            VarSNaN <= '1';
            VarQNaN <= '0';
		
		else
		
			VarSNaN <= '0';
			VarQNaN <= '0';
			
		end if;
		
	end process;
	
	
	
	check_Inf : process(VarA_2,VarB_2,VarC_2)
	
	begin
		
		if (unsigned(VarA_2(63 downto 52)) = 4095 and unsigned(VarA_2(51 downto 0)) = 0)
			or (unsigned(VarB_2(63 downto 52)) = 4095 and unsigned(VarB_2(51 downto 0)) = 0)
			or (unsigned(VarC_2(63 downto 52)) = 4095 and unsigned(VarC_2(51 downto 0)) = 0) then
			
			VarInf <= '1';
		
		else
		
			VarInf <= '0';
			
		end if;

	end process;


	
	InitialMultiply : process(Sign_A,Sign_B,Sign_C,Exponent_A,Exponent_B,Exponent_C,Significand_A,Significand_B,Significand_C)
		
	variable Exponent_Swap : natural range 0 to 8191;	
		
	begin
		
			Sign_Multiply <= Sign_A xor Sign_B;
			Exponent_Swap := Exponent_A + Exponent_B;
			if Exponent_A = 4095 or Exponent_B = 4095 then
			    Exponent_Multiply <= 4095;
			elsif Exponent_Swap > 2047 then
			    Exponent_Multiply <= Exponent_Swap - 2047;	
			else
			    Exponent_Multiply <= 0;
			end if;
			Significand_Multiply <= '0' & std_logic_vector(unsigned(Significand_A) * unsigned(Significand_B)) & x"00000000000000";	

			Sign_Addition <= Sign_C;
			Exponent_Addition <= Exponent_C;	
			Significand_Addition <= "00" & Significand_C & x"000000000000000000000000000";

	end process;
	
	
	
	
	
	MultStage : process (CLK,nRST)
	begin
		if nRST='0' then
			sigReady_2 <= '0';
			varFMT_2 <= (others => '0');
			VarSNaN_2 <= '0';
			VarQNaN_2 <= '0';
			VarInf_2 <= '0';
			VarRM_2 <= (others => '0');
			VarFMAOP_2 <= (others => '0');
			
			Sign_Multiply_2 <= '0';
			Exponent_Multiply_2 <= 0;
			Significand_Multiply_2 <= (others => '0');
			Sign_Addition_2 <= '0';
			Exponent_Addition_2 <= 0;
			Significand_Addition_2 <= (others => '0');
		elsif rising_edge(CLK) then
			sigReady_2 <= sigReady;
			varFMT_2 <= varFMT;
			VarSNaN_2 <= VarSNaN;
			VarQNaN_2 <= VarQNaN;
			VarInf_2 <= VarInf;
			VarRM_2 <= VarRM;
			VarFMAOP_2 <= VarFMAOP;
			
			Sign_Multiply_2 <= Sign_Multiply;
			Exponent_Multiply_2 <= Exponent_Multiply;
			Significand_Multiply_2 <= Significand_Multiply;
			Sign_Addition_2 <= Sign_Addition;
			Exponent_Addition_2 <= Exponent_Addition;
			Significand_Addition_2 <= Significand_Addition;
		end if;
	end process;
	
	
	
	PreAddition : process (Exponent_Multiply_2,Significand_Multiply_2,Exponent_Addition_2,Significand_Addition_2)
	
	variable Exponent_Difference : natural range 0 to 8191;
	variable Shift_Counter       : natural range 0 to 255;
	
	begin
		
		Exponent_Difference := 0;
		
		if Exponent_Multiply_2 > Exponent_Addition_2 then
			
			Exponent_Difference := Exponent_Multiply_2 - Exponent_Addition_2;
			
			if Exponent_Difference > 108 then
				Shift_Counter := 108;
			else
				Shift_Counter := Exponent_Difference;
			end if;
			
			Significand_Multiply_3 <= Significand_Multiply_2;
			Significand_Addition_3 <= std_logic_vector(shift_right(unsigned(Significand_Addition_2),Shift_Counter));
			
			Exponent_Add <= Exponent_Multiply_2;
			
		else
			
			Exponent_Difference := Exponent_Addition_2 - Exponent_Multiply_2;
			
			if Exponent_Difference > 56 then
				Shift_Counter := 56;
			else
				Shift_Counter := Exponent_Difference;
			end if;
			
			Significand_Addition_3 <= Significand_Addition_2;
			Significand_Multiply_3 <= std_logic_vector(shift_right(unsigned(Significand_Multiply_2),Shift_Counter));
			
			Exponent_Add <= Exponent_Addition_2;
			
		end if;
		
		
	end process;
	
	
	
	
	Addition : process(Significand_Multiply_3,Significand_Addition_3,Sign_Addition_2,Sign_Multiply_2)
	
	variable VarSignificand_Left  : std_logic_vector(162 downto 0);
	variable VarSignificand_Right : std_logic_vector(162 downto 0);
	
	begin
		
		if Sign_Addition_2 = Sign_Multiply_2 then
			
			Significand_Add <= std_logic_vector(unsigned(Significand_Addition_3)+unsigned(Significand_Multiply_3));
			
			Sign_Add <= Sign_Addition_2;
			
		else
			
			if Significand_Multiply_3 = Significand_Addition_3 then
			
				Significand_Add <= (others => '0');
				
				Sign_Add <= '0';
			
			else
			
				if unsigned(Significand_Multiply_3)>unsigned(Significand_Addition_3) then
				
					Sign_Add <= Sign_Multiply_2;
					
					VarSignificand_Left := Significand_Multiply_3;
					VarSignificand_Right := Significand_Addition_3;
					
				else
				
					Sign_Add <= Sign_Addition_2;
					
					VarSignificand_Left := Significand_Addition_3;
					VarSignificand_Right := Significand_Multiply_3;
					
				end if;
				
				Significand_Add <= std_logic_vector(unsigned(VarSignificand_Left)-unsigned(VarSignificand_Right));
			
			end if;
			
		end if;
		
	end process;

	
	
	
	LZCounter161Bit : entity work.LZCounter161Bit(Behavioral) 
		port map (
			A => Significand_Add(160 downto 0),
			Z => Counter_Add
		);

	
	
	
	AddStage : process (CLK,nRST)
	begin
		if nRST='0' then
			sigReady_3 <= '0';
			varFMT_3 <= (others => '0');
			VarSNaN_3 <= '0';
			VarQNaN_3 <= '0';
			VarInf_3 <= '0';
			VarRM_3 <= (others => '0');
			VarFMAOP_3 <= (others => '0');

			Sign_Add_2 <= '0';
			Exponent_Add_2 <= 0;
			Significand_Add_2 <= (others => '0');
			Counter_Add_2 <= (others => '0');
		elsif rising_edge(CLK) then
			sigReady_3 <= sigReady_2; 
			varFMT_3 <= varFMT_2;
			VarSNaN_3 <= VarSNaN_2;
			VarQNaN_3 <= VarQNaN_2;
			VarInf_3 <= VarInf_2;
			VarRM_3 <= VarRM_2;
			VarFMAOP_3 <= VarFMAOP_2;

			Sign_Add_2 <= Sign_Add;
			Exponent_Add_2 <= Exponent_Add;
			Significand_Add_2 <= Significand_Add;
			Counter_Add_2 <= Counter_Add;
		end if;
	end process;
	
	
	
	
	

	Normalisation : process (Sign_Add_2,Exponent_Add_2,Counter_Add_2,Significand_Add_2)
	
	variable CounterAdd : natural range 0 to 255;
	
	begin
			
		
		
		if Significand_Add_2(162) = '1' then
			
			VarSign <= Sign_Add_2;
			VarExponent <= Exponent_Add_2+2;
			VarSignificand <= std_logic_vector(shift_right(unsigned(Significand_Add_2),2));
		
		elsif Significand_Add_2(161) = '1' then
		
			VarSign <= Sign_Add_2;
			VarExponent <= Exponent_Add_2+1;
			VarSignificand <= std_logic_vector(shift_right(unsigned(Significand_Add_2),1));

		elsif Significand_Add_2(160) = '0' then
		
			VarSign <= Sign_Add_2;
			CounterAdd := to_integer(unsigned(Counter_Add_2));
			if CounterAdd > Exponent_Add_2 then
			    CounterAdd := Exponent_Add_2;
			end if;
			VarExponent <= Exponent_Add_2 - CounterAdd;
			VarSignificand <= std_logic_vector(shift_left(unsigned(Significand_Add_2),CounterAdd));
			
		else

			VarSign <= Sign_Add_2;
			VarExponent <= Exponent_Add_2;
			VarSignificand <= Significand_Add_2;
		
		end if;
	
	end process;
	
	
	
	
	
	Rounding : process (VarSign,VarExponent,VarSignificand,VarSNaN_3,VarQNaN_3,VarInf_3,VarRM_3,VarFMAOP_3,varFMT_3,sigReady_3)

	variable VarSign_Round : std_logic;
	variable VarSignificand_Conversion : std_logic_vector(217 downto 0);
	variable VarSignificand_Round : std_logic_vector(53 downto 0);
	variable VarExponent_Round : integer range -8191 to 8191;
	variable VarEX : std_logic_vector(4 downto 0);
	
	variable G : std_logic;
	variable R : std_logic;
	variable S : std_logic;
	variable GRS : std_logic;
	variable Even : std_logic;
	
	variable Round : boolean;

		
	begin

		GRS := '0';
		VarEX :=  (others => '0');
		Round := False;
		
		if VarFMAOP_3 = "010" then
			VarSign_Round := not VarSign;
		elsif VarFMAOP_3 = "011" then
			VarSign_Round := not VarSign;
		else
			VarSign_Round := VarSign;
		end if;
			
		VarSignificand_Conversion := VarSignificand(161 downto 0) & x"00000000000000";
		
		if varFMT_3 = "00" then

			VarExponent_Round := VarExponent - 1920;
			
			if unsigned(VarSignificand_Conversion) = 0 then
				VarExponent_Round := 0;
			elsif VarExponent_Round < 1 and VarExponent_Round > -26 then
				VarSignificand_Conversion := std_logic_vector(shift_right(unsigned(VarSignificand_Conversion),1-VarExponent_Round));
				VarExponent_Round := 0;
			elsif VarExponent_Round <= -26 then
				VarExponent_Round := -1;
				VarSignificand_Conversion := std_logic_vector(shift_right(unsigned(VarSignificand_Conversion),26));
			end if;
			
			VarSignificand_Round := "0" & x"0000000" & VarSignificand_Conversion(217 downto 193);
			
			G := VarSignificand_Conversion(192);
			R := VarSignificand_Conversion(191);
			S := or_reduce(VarSignificand_Conversion(190 downto 0));
				
			GRS := G or R or S;
			Even := VarSignificand_Round(0) or R or S;
			
			if GRS = '1' then
				VarEX(0) := '1';
			end if;

			if VarSignificand_Conversion(215 downto 192) = x"FFFFFF" and R = '0' and VarExponent_Round = 0 then
				VarEX(1) := GRS;
			end if;
			
			case VarRM_3 is
						
				when "000" => --RNE--
					if G='1' and Even = '1' then
						Round := True;
					end if;
				when "001" => --RTZ--
				
				when "010" => --RDN--
					if VarSign_Round = '1' and GRS = '1' then
						Round := True;
					end if;
				
				when "011" => --RUP--
					if VarSign_Round = '0' and GRS = '1' then
						Round := True;
					end if;
				
				when "100" => --RMM--
					if GRS = '1' then
						Round := True; 
					end if;
				
				when others =>
				
			end case;
			
			if Round then
				VarSignificand_Round := std_logic_vector(unsigned(VarSignificand_Round) + 1); 
			end if;
			
			if VarSignificand_Round(24) = '1' then
				VarExponent_Round := VarExponent_Round+1;
				VarSignificand_Round := std_logic_vector(shift_right(unsigned(VarSignificand_Round),1));
			end if;
			
			if VarSignificand_Round(23) = '1' and VarExponent_Round = 0 then
			    VarExponent_Round := 1;
			end if;
			
			if VarSignificand_Round(23) = '0' and VarExponent_Round = 0 then
				VarEX(1) := GRS;
			end if;
			
			if VarSNaN_3 = '1' then
				VarEX := "10000";
				P <= x"FFFFFFFF" & x"7FC00000";
			elsif VarQNaN_3 = '1' then
				VarEX := "00000";
				P <= x"FFFFFFFF" & x"7FC00000";
			elsif VarInf_3 = '1' then
				VarEX := "00000";
				P <= x"FFFFFFFF" & VarSign_Round & x"FF" & "000" & x"00000";
			elsif VarExponent_Round < 0 then
				VarEX(1) := GRS;
				P <= x"FFFFFFFF" & VarSign_Round & x"00" & "000" & x"00000";
			elsif VarExponent_Round > 254 then
				VarEX(2) := '1';
				VarEX(0) := '1';
				P <= x"FFFFFFFF" & VarSign_Round & x"FF" & "000" & x"00000";
			else
				P <= x"FFFFFFFF" & VarSign_Round & std_logic_vector(to_unsigned(VarExponent_Round,8)) & VarSignificand_Round(22 downto 0);
			end if;

		elsif varFMT_3 = "01" then
		
			VarExponent_Round := VarExponent - 1024;

			if unsigned(VarSignificand_Conversion) = 0 then
				VarExponent_Round := 0;
			elsif VarExponent_Round < 1 and VarExponent_Round > -55 then
				VarSignificand_Conversion := std_logic_vector(shift_right(unsigned(VarSignificand_Conversion),1-VarExponent_Round));
				VarExponent_Round := 0;
			elsif VarExponent_Round <= -55 then
				VarExponent_Round := -1;
				VarSignificand_Conversion := std_logic_vector(shift_right(unsigned(VarSignificand_Conversion),55));
			end if;
			
			VarSignificand_Round := VarSignificand_Conversion(217 downto 164);
			
			G := VarSignificand_Conversion(163);
			R := VarSignificand_Conversion(162);
			S := or_reduce(VarSignificand_Conversion(161 downto 0));
				
			GRS := G or R or S;
			Even := VarSignificand_Round(0) or R or S;

			if GRS = '1' then
				VarEX(0) := '1';
			end if;

			if VarSignificand_Conversion(215 downto 164) = x"FFFFFFFFFFFFF" and G = '1' and R = '0' and VarExponent_Round = 0 then
				VarEX(1) := GRS;
			end if;
			
			case VarRM_3 is
						
				when "000" => --RNE--
					if G='1' and Even = '1' then
						Round := True; 
					end if;
				when "001" => --RTZ--
				
				when "010" => --RDN--
					if VarSign_Round = '1' and GRS = '1' then
						Round := True;
					end if;
				
				when "011" => --RUP--
					if VarSign_Round = '0' and GRS = '1' then
						Round := True;
					end if;
				
				when "100" => --RMM--
					if GRS = '1' then
						Round := True;
					end if;
				
				when others =>
				
			end case;
			
			if Round then
				VarSignificand_Round := std_logic_vector(unsigned(VarSignificand_Round) + 1); 
			end if;
			
			if VarSignificand_Round(53) = '1' then
				VarExponent_Round := VarExponent_Round+1;
				VarSignificand_Round := std_logic_vector(shift_right(unsigned(VarSignificand_Round),1));
			end if;
			
			if VarSignificand_Round(52) = '1' and VarExponent_Round = 0 then
				VarExponent_Round := 1;
			end if;
			
			if VarSignificand_Round(52) = '0' and VarExponent_Round = 0 then
				VarEX(1) := GRS;
			end if;
			
			if VarSNaN_3 = '1' then
				VarEX := "10000";
				P <= x"7FF8000000000000";
			elsif VarQNaN_3 = '1' then
				VarEX := "00000";
				P <= x"7FF8000000000000";
			elsif VarInf_3 = '1' then
				VarEX := "00000";
				P <= VarSign_Round & "111" & x"FF" & x"0000000000000";
			elsif VarExponent_Round < 0 then
				VarEX(1) := GRS;
				P <= VarSign_Round & "000" & x"00" & x"0000000000000";
			elsif VarExponent_Round > 2046 then
				VarEX(2) := '1';
				VarEX(0) := '1';
				P <= VarSign_Round & "111" & x"FF" & x"0000000000000";
			else
				P <= VarSign_Round & std_logic_vector(to_unsigned(VarExponent_Round,11)) &  VarSignificand_Round(51 downto 0);
			end if;

		end if;

		Ex <= VarEX;
		Ready <= sigReady_3;
		
		
	end process;

end Behavioral;
