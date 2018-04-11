library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;


entity MultiplicationStage is
	Port(
		multiplicand_0		: in  std_logic_vector(63 downto 0);
		multiplicand_1		: in  std_logic_vector(63 downto 0);
		extended				: in  std_logic;
		multiply				: in  std_logic;
		alu32					: in  std_logic;
		funct					: in  std_logic_vector(2 downto 0);
		nRST					: in  std_logic;
		CLK					: in  std_logic;
		result				: out std_logic_vector(63 downto 0);
		ready					: out std_logic
	);
end MultiplicationStage;


architecture Behavioral of MultiplicationStage is

signal s_mult_0 : std_logic_vector(63 downto 0);
signal s_mult_1 : std_logic_vector(63 downto 0);
signal s_mult_0_p : std_logic_vector(63 downto 0);
signal s_mult_1_p : std_logic_vector(63 downto 0);

signal s_res_00 : std_logic_vector(127 downto 0);
signal s_res_01 : std_logic_vector(127 downto 0); 
signal s_res_10 : std_logic_vector(127 downto 0); 
signal s_res_11 : std_logic_vector(127 downto 0);
signal s_res_00_p : std_logic_vector(127 downto 0);
signal s_res_01_p : std_logic_vector(127 downto 0); 
signal s_res_10_p : std_logic_vector(127 downto 0); 
signal s_res_11_p : std_logic_vector(127 downto 0);

signal s_res_0 : std_logic_vector(127 downto 0);
signal s_res_1 : std_logic_vector(127 downto 0);
signal s_res_0_p : std_logic_vector(127 downto 0);
signal s_res_1_p : std_logic_vector(127 downto 0);

signal s_res : std_logic_vector(127 downto 0);
signal s_res_p : std_logic_vector(127 downto 0);



signal s_sign : std_logic;
signal s_ready : std_logic;
signal s_alu32 : std_logic;
signal s_funct : std_logic_vector(2 downto 0);



signal s_sign_2 : std_logic;
signal s_ready_2 : std_logic;
signal s_alu32_2 : std_logic;
signal s_funct_2 : std_logic_vector(2 downto 0);



signal s_sign_3 : std_logic;
signal s_ready_3 : std_logic;
signal s_alu32_3 : std_logic;
signal s_funct_3 : std_logic_vector(2 downto 0);



signal s_sign_4 : std_logic;
signal s_ready_4 : std_logic;
signal s_alu32_4 : std_logic;
signal s_funct_4 : std_logic_vector(2 downto 0);



signal s_sign_5 : std_logic;
signal s_ready_5 : std_logic;
signal s_alu32_5 : std_logic;
signal s_funct_5 : std_logic_vector(2 downto 0);
 

begin

	Initial : process (multiplicand_0,multiplicand_1,multiply,alu32,funct)
	
	variable varsign : std_logic;
	variable varmult_0 : std_logic_vector(63 downto 0);
	variable varmult_1 : std_logic_vector(63 downto 0);
	
	begin
			
		varsign := '0';
		varmult_0 := (others => '0');
		varmult_1 := (others => '0');
		
		if multiply = '0' then
			
			s_ready <= '0';
			s_alu32 <= '0';
			s_funct <= (others => '0');
		
		else
		
			varsign := '0';
			varmult_0 := multiplicand_0;
			varmult_1 := multiplicand_1;
		
			if alu32 = '1' then
			
				if funct = FUNC_MULHSU then
					varsign := multiplicand_0(31);
				elsif funct /= FUNC_MULHU then
					varsign := multiplicand_0(31) xor multiplicand_1(31);
				else
					varsign := '0';
				end if;
				
				
				if funct /= FUNC_MULHU   then
				
					if multiplicand_0(31) = '1' then
						varmult_0(31 downto 0) := std_logic_vector(unsigned(not multiplicand_0(31 downto 0))+1);
					end if;
				
					if funct /= FUNC_MULHSU then
					
						if multiplicand_1(31) = '1' then
							varmult_1(31 downto 0) := std_logic_vector(unsigned(not multiplicand_1(31 downto 0))+1);
						end if;
					
					end if;
				
				end if;
				
				varmult_0(63 downto 32) := (others => '0');
				varmult_1(63 downto 32) := (others => '0');
				
			else
			
				if funct = FUNC_MULHSU then
					varsign := multiplicand_0(63);
				elsif funct /= FUNC_MULHU then
					varsign := multiplicand_0(63) xor multiplicand_1(63);
				else
					varsign := '0';
				end if;
				
				if funct /= FUNC_MULHU   then
				
					if multiplicand_0(63) = '1' then
						varmult_0 := std_logic_vector(unsigned(not multiplicand_0)+1);
					end if;
					
					if funct /= FUNC_MULHSU then
					
						if multiplicand_1(63) = '1' then
							varmult_1 := std_logic_vector(unsigned(not multiplicand_1)+1);
						end if;
						
					end if;
					
				end if;
				
			end if;
			
			
			s_ready <= '1';
			s_alu32 <= alu32;
			s_funct <= funct;
			
		end if;
		
		s_sign <= varsign;
		s_mult_0 <= varmult_0;
		s_mult_1 <= varmult_1;
		
	end process;
	
	
		
		
		
	process (CLK,nRST)
		
	begin
		
		if nRST = '0' then
		
			s_sign_2 <= '0';
			s_ready_2 <= '0';
			s_alu32_2 <= '0';
			s_funct_2 <= (others => '0');
			s_mult_0_p <= (others => '0');
			s_mult_1_p <= (others => '0');
		
		elsif rising_edge(CLK) then
		
			s_sign_2 <= s_sign;
			s_ready_2 <= s_ready;
			s_alu32_2 <= s_alu32;
			s_funct_2 <= s_funct;
			s_mult_0_p <= s_mult_0;
			s_mult_1_p <= s_mult_1;
		
		end if;
		
	end process;
	
	
	
	
	Mult : process (s_mult_0_p,s_mult_1_p)
	
	begin
	
		s_res_00 <= x"0000000000000000" & std_logic_vector(unsigned(s_mult_0_p(31 downto 0)) * unsigned(s_mult_1_p(31 downto 0)));
		s_res_01 <= x"00000000" & std_logic_vector(unsigned(s_mult_0_p(31 downto 0)) * unsigned(s_mult_1_p(63 downto 32))) & x"00000000";
		s_res_10 <= x"00000000" & std_logic_vector(unsigned(s_mult_0_p(63 downto 32)) * unsigned(s_mult_1_p(31 downto 0))) & x"00000000";
		s_res_11 <= std_logic_vector(unsigned(s_mult_0_p(63 downto 32)) * unsigned(s_mult_1_p(63 downto 32))) & x"0000000000000000";
	
	end process;
	
	
	
		
		
		
	process (CLK,nRST)
		
	begin
		
		if nRST = '0' then
		
			s_sign_3 <= '0';
			s_ready_3 <= '0';
			s_alu32_3 <= '0';
			s_funct_3 <= (others => '0');
			s_res_00_p <= (others => '0');
			s_res_01_p <= (others => '0');
			s_res_10_p <= (others => '0');
			s_res_11_p <= (others => '0');
		
		elsif rising_edge(CLK) then
		
			s_sign_3 <= s_sign_2;
			s_ready_3 <= s_ready_2;
			s_alu32_3 <= s_alu32_2;
			s_funct_3 <= s_funct_2;
			s_res_00_p <= s_res_00;
			s_res_01_p <= s_res_01;
			s_res_10_p <= s_res_10;
			s_res_11_p <= s_res_11;
		
		end if;
		
	end process;
	
	
	AddPre : process (s_res_00_p,s_res_01_p,s_res_10_p,s_res_11_p)
	
	begin
	
		s_res_0 <= std_logic_vector(unsigned(s_res_00_p) + unsigned(s_res_01_p));
		s_res_1 <= std_logic_vector(unsigned(s_res_10_p) + unsigned(s_res_11_p));
	
	end process;
	
	
	
		
		
		
	process (CLK,nRST)
		
	begin
		
		if nRST = '0' then
		
			s_sign_4 <= '0';
			s_ready_4 <= '0';
			s_alu32_4 <= '0';
			s_funct_4 <= (others => '0');
			s_res_0_p <= (others => '0');
			s_res_1_p <= (others => '0');
		
		elsif rising_edge(CLK) then
		
			s_sign_4 <= s_sign_3;
			s_ready_4 <= s_ready_3;
			s_alu32_4 <= s_alu32_3;
			s_funct_4 <= s_funct_3;
			s_res_0_p <= s_res_0;
			s_res_1_p <= s_res_1;
		
		end if;
		
	end process;
	
	
	AddPost : process (s_res_0_p,s_res_1_p)
	
	begin
	
		s_res <= std_logic_vector(unsigned(s_res_0_p) + unsigned(s_res_1_p));
	
	end process;
	
	
	
		
		
		
	process (CLK,nRST)
		
	begin
		
		if nRST = '0' then
		
			s_sign_5 <= '0';
			s_ready_5 <= '0';
			s_alu32_5 <= '0';
			s_funct_5 <= (others => '0');
			s_res_p <= (others => '0');
		
		elsif rising_edge(CLK) then
		
			s_sign_5 <= s_sign_4;
			s_ready_5 <= s_ready_4;
			s_alu32_5 <= s_alu32_4;
			s_funct_5 <= s_funct_4;
			s_res_p <= s_res;
		
		end if;
		
	end process;
	
	
	Output : process (s_res_p,s_sign_5,s_ready_5,s_alu32_5,s_funct_5)
	
	variable varmult_e : std_logic_vector(127 downto 0);
	variable varresult : std_logic_vector(63 downto 0);
	
	begin
	
		varmult_e := (others => '0');
		varresult := (others => '0');
		
		if s_ready_5 = '1' then
			
			if s_sign_5 = '1' then
				varmult_e := std_logic_vector(unsigned(not s_res_p)+1);
			else
				varmult_e := s_res_p;
			end if;
			
			if s_funct_5 = FUNC_MUL then
			
				if s_alu32_5 = '1' then
					varresult(31 downto 0) := varmult_e(31 downto 0);
				else
					varresult := varmult_e(63 downto 0);
				end if;
			
			elsif unsigned(s_funct_5) < 4 then
			
				varresult := varmult_e(127 downto 64);
			
			end if;
		
		end if;
		
		ready <= s_ready_5;
		result <=  varresult;
		
	end process;
	
	
end;
