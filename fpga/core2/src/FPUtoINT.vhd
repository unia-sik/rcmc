library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;
use work.libeu.all;


entity FPUtoINT is
    Port ( 
        FPU : in std_logic_vector(63 downto 0);
        FMT : in std_logic_vector(1 downto 0);
        Funct : in std_logic_vector(6 downto 0);
        RM : in std_logic_vector(2 downto 0);
        FPUOP : in  std_logic;
        nRST : in std_logic;
        CLK : in  std_logic;
        ----------------------------------------------------------
        Ex : out std_logic_Vector(4 downto 0);
        INT : out std_logic_vector(63 downto 0);
        Ready : out  std_logic
    );
end FPUtoINT;

architecture Behavioral of FPUtoINT is


signal sigINT : std_logic_vector(64 downto 0);
signal sigINT_2 : std_logic_vector(64 downto 0);

signal sigSignificand : std_logic_vector(54 downto 0);
signal sigSignificand_2 : std_logic_vector(54 downto 0);

signal sigOoR : std_logic;
signal sigOoR_2 : std_logic;


signal sigSign : std_logic;
signal sigSign_2 : std_logic;

signal sigNaN : std_logic;
signal sigNaN_2 : std_logic;

signal sigReady : std_logic;
signal sigReady_2 : std_logic;

signal sigEX : std_logic_vector(4 downto 0);
signal sigEX_2 : std_logic_vector(4 downto 0);

signal sigRM : std_logic_vector(2 downto 0);
signal sigFMT : std_logic_vector(1 downto 0);

signal sigCounter : integer range -63 to 63;
signal sigCounter_2 : integer range -63 to 63;


begin
	process (FPU,RM,Funct,FMT,FPUOP)
	
	variable varINT : std_logic_vector(64 downto 0);
	variable VarNaN : std_logic;
	variable VarEX : std_logic_vector(4 downto 0);
	variable VarSign : std_logic;
	variable VarExponent : std_logic_vector(10 downto 0);
	variable VarSignificand : std_logic_vector(118 downto 0);
	
	variable VarOoR : std_logic;
	
	variable VarCounter : integer range -63 to 63;
	
	
	begin
		
		varINT := (others => '0');
		VarNaN := '0';
		VarEX := (others => '0');
		VarSign := '0';
		VarExponent := (others => '0');
		VarSignificand := (others => '0');
		
		VarOoR := '0';
		
		VarCounter := 0;
		
		
		
			
		if FPUOP = '0' then
			
			sigINT <= (others => '0');
			sigNaN <= varNaN;
			sigSignificand <= (others => '0');
			sigOoR <= '0';
			sigSign <= '0';
			sigEX <= (others => '0');
			
			sigReady <= '0';
			
		
		elsif Funct(6 downto 1) = "110000" then
		
			if Funct(0) = '0' then
			
				VarSign := FPU(31);
				VarExponent(7 downto 0) := FPU(30 downto 23);
				
				if unsigned(VarExponent) = 255 and unsigned(FPU(22 downto 0)) /= 0 then
				
					VarNaN := '1';
					VarEX(4) := '1';
				
				elsif unsigned(VarExponent) > 123 and unsigned(VarExponent) < 191  then

					VarSignificand(55) := '1';
					VarSignificand(54 downto 32) := FPU(22 downto 0);
					VarCounter := to_integer(unsigned(VarExponent))-127;
				
				elsif unsigned(VarExponent) >= 191 then
				
					VarOoR := '1';
				
				end if;
				
			else
			
				VarSign := FPU(63);
				VarExponent := FPU(62 downto 52);
				
				if unsigned(VarExponent) = 2047 and unsigned(FPU(51 downto 0)) /= 0 then
				
					VarNaN := '1';
					VarEX(4) := '1';
				
				elsif unsigned(VarExponent) > 1019 and unsigned(VarExponent) < 1087  then

					VarSignificand(55) := '1';
					VarSignificand(54 downto 3) := FPU(51 downto 0);
					VarCounter := to_integer(unsigned(VarExponent))-1023;	
				
				elsif unsigned(VarExponent) >= 1087 then
					
					VarOoR := '1';
				
				end if;
			
			end if;
			
			
			if VarCounter > 0 then
			
				VarSignificand := std_logic_vector(shift_left(unsigned(VarSignificand),VarCounter));
				
			elsif VarCounter < 0 then
			
				VarSignificand := std_logic_vector(shift_right(unsigned(VarSignificand),(-VarCounter)));
				
			end if;
			
			if VarOoR = '0' then
			
				varINT(63 downto 0) := VarSignificand(118 downto 55);
			
			else
			
				varINT := (others => '1');
			
			end if;
			
			
			sigINT <= varINT;
			sigNaN <= varNaN;
			sigSignificand <= VarSignificand(54 downto 0);
			sigOoR <= VarOoR;
			sigSign <= VarSign;
			sigEX <= VarEX;
			sigCounter <= varCounter;
			
			sigReady <= '1';
				
		else
			
			sigINT <= (others => '0');
			sigNaN <= varNaN;
			sigSignificand <= (others => '0');
			sigOoR <= '0';
			sigSign <= '0';
			sigEX <= (others => '0');	
			sigCounter <= 0;	
			
			sigReady <= '0';
		
		end if;

	end process;
	
	
	process (CLK,nRST)
	
	begin 
	
		if nRST = '0' then
		
			sigReady_2 <= '0';
			sigINT_2 <= (others => '0');
			sigNaN_2 <= '0';
			sigSignificand_2 <= (others => '0');
			sigOoR_2 <= '0';
			sigSign_2 <= '0';
			sigCounter_2 <= 0;	
			sigEX_2 <= (others => '0');
			sigRM <= (others => '0');
			sigFMT <= (others => '0');
	
		elsif rising_edge(CLK) then
        
			sigReady_2 <= sigReady;
			sigINT_2 <= sigINT;
			sigNaN_2 <= sigNaN;
			sigSignificand_2 <= sigSignificand;
			sigOoR_2 <= sigOoR;
			sigSign_2 <= sigSign;
			sigCounter_2 <= sigCounter;	
			sigEX_2 <= sigEX;
			sigEX_2 <= sigEX;
			sigRM <= RM;
			sigFMT <= FMT;
		
		end if;
		
	end process;
		
		
	process (sigINT_2,sigNaN_2,sigSignificand_2,sigOoR_2,sigSign_2,sigCounter_2,sigEX_2,sigRM,sigFMT,sigReady_2)
	
	variable varINT : std_logic_vector(64 downto 0);
	variable VarSignificand : std_logic_vector(54 downto 0);
	variable VarOoR : std_logic;
	variable VarSign : std_logic;
	variable VarEX : std_logic_vector(4 downto 0);
	
	variable G : std_logic;
	variable R : std_logic;
	variable S : std_logic;
	variable GRS : std_logic;
	variable Even : std_logic;
	
	variable OR_1 : std_logic;
	variable OR_2 : std_logic;
	variable OR_3 : std_logic;
	variable OR_4 : std_logic;
	variable OR_5 : std_logic;
	
	
	variable BED1 : std_logic;
	variable BED2 : std_logic;
	
	variable VarCounter : integer range -63 to 63;
	
	
	begin
	
		VarSignificand := sigSignificand_2;
		VarCounter := sigCounter_2;
		varINT := sigINT_2;
		VarOoR := sigOoR_2;
		VarSign := sigSign_2;
		VarEX := sigEX_2;
		GRS := '0';
		
		
		
		G := VarSignificand(54);
		R := VarSignificand(53);
		S := or_reduce(VarSignificand(52 downto 0));
		
		GRS := G or R or S;
		
		Even := varINT(0) or R or S;

		VarEX(0) := GRS;
			
		case sigRM is
				
			when "000" => --RNE--
			
				if G='1' and Even = '1' then
					varINT := std_logic_vector(unsigned(varINT) + 1); 
				end if;
				
			when "001" => --RTZ--
			
			when "010" => --RDN--
			
				if VarSign = '1' and GRS = '1' then
					varINT := std_logic_vector(unsigned(varINT) + 1); 
				end if;
			
			when "011" => --RUP--
			
				if VarSign = '0' and GRS = '1' then
					varINT := std_logic_vector(unsigned(varINT) + 1); 
				end if;
			
			when "100" => --RMM--
			
				if GRS = '1' then
					varINT := std_logic_vector(unsigned(varINT) + 1);  
				end if;
			
			when others =>
			
		end case;
		
		
		OR_1 := varINT(64);
		OR_2 := varINT(63);
		OR_3 := or_reduce(varINT(62 downto 32));
		OR_4 := varINT(31);
		OR_5 := or_reduce(varINT(30 downto 0));
		
		if sigFMT = "00" then
			
			if VarSign = '0' and (OR_1 = '1' or OR_2 = '1' or OR_3 = '1' or OR_4 = '1') then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			elsif VarSign = '1' and (OR_1 = '1' or OR_2 = '1' or OR_3 = '1') then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			elsif VarSign = '1' and (OR_1 = '0' and OR_2 = '0' and OR_3 = '0' and OR_4 = '1' and OR_5 = '1' ) then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			end if;
			
		elsif sigFMT = "01" then
		
			if VarSign = '0' and (OR_1 = '1' or OR_2 = '1' or OR_3 = '1') then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			elsif VarSign = '1' and (OR_1 = '1' or OR_2 = '1' or OR_3 = '1') then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			elsif VarSign = '1' and OR_1 = '0' and OR_2 = '0' and OR_3 = '0' and (OR_4 = '1' or OR_5 = '1' ) then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			end if;
		
		elsif sigFMT = "10" then
		
			if VarSign = '0' and (OR_1 = '1' or OR_2 = '1') then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			elsif VarSign = '1' and (OR_1 = '1') then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			elsif VarSign = '1' and (OR_1 = '0' and OR_2 = '1' and (OR_3 = '1' or OR_4 = '1' or OR_5 = '1') ) then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			end if;
		
		else
		
			if VarSign = '0' and OR_1 = '1' then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			elsif VarSign = '1'and OR_1 = '1' then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '0');
			elsif VarSign = '1' and OR_1 = '0' and (OR_2 = '1' or OR_3 = '1' or OR_4 = '1' or OR_5 = '1') then
				VarOoR := '1';
				VarEX(4) := '1'; 
				varINT := (others => '1');
			end if;
		
		end if;
		
		
		if sigNaN_2 = '1' then
		
			if sigFMT = "00" then
			
				VarINT(63 downto 31) := (others => '0');
				VarINT(30 downto 0) := (others => '1');
				
			elsif sigFMT = "01" then
			
				VarINT(63 downto 32) := (others => '1');
				VarINT(31 downto 0) := (others => '1');
			
			elsif sigFMT = "10" then
			
				VarINT(63) := '0';
				VarINT(62 downto 0) := (others => '1');
			
			else
			
				VarINT(63 downto 0) := (others => '1');
			
			end if;
		
		else
		
			if sigFMT = "00" then
					
				if VarSign = '0' then
				
					VarINT(63 downto 31) := (others => '0');
					
				else
					
					if VarOoR = '0' then
					
					    VarINT(31 downto 0) := std_logic_vector(unsigned(not VarINT(31 downto 0))+1);
					
					    VarINT(63 downto 32) := (others => VarINT(31));
					
					else
					
					    VarINT(63 downto 32) := (others => '1');
					
						VarINT(31 downto 0) := (31 => '1', others => '0');
					
					end if;
				
					
				end if;
			
			elsif sigFMT = "01" then
					
				if VarSign = '0' then
					
					VarINT(63 downto 32) := (others => varINT(31));
				
				else
				
					VarINT(63 downto 0) := (others => '0');
					
				end if;
				
			elsif sigFMT = "10" then
					
				if VarSign = '0' then
				
					VarINT(63) := '0';
					
				else
					
					if VarOoR = '0' then
						
						VarINT := std_logic_vector(unsigned(not VarINT)+1);
					
					else
						
						VarINT := (63 => '1', others => '0');
					
					end if;
					
				end if;
				
			else
					
				if VarSign = '1' then
				
					VarINT := (others => '0');
					
				end if;
			
			end if;
		
		end if;
		
		INT <= varINT(63 downto 0);
		Ready <= sigReady_2;
		EX <= VarEX;
		
		
	end process;

end Behavioral;
