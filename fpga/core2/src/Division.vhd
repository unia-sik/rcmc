library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.constants.all;

entity Division is
	Port( 
		dividend		: in  std_logic_vector(63 downto 0);
		divisor		: in  std_logic_vector(63 downto 0);
		funct			: in  std_logic_vector(2 downto 0);
		alu32			: in  std_logic;
		extended		: in  std_logic;
		division		: in  std_logic;
		remaind		: in  std_logic;
		nRST			: in  std_logic;
		CLK			: in  std_logic;
		quotient		: out std_logic_vector(63 downto 0);
		remainder	: out std_logic_vector(63 downto 0);
		ready			: out std_logic
	);
end Division;


architecture Behavioral of Division is

signal s_alu32      : std_logic;
signal s_alu32_2    : std_logic;
signal s_alu32_3    : std_logic;
signal s_alu32_4    : std_logic;
signal s_alu32_5    : std_logic;
signal s_alu32_swap : std_logic;

	
signal dd   : unsigned(127 downto 0);   -- dividend
signal dr   : unsigned(63 downto 0);    -- divisor
signal q    : unsigned(63 downto 0);    -- qoutient
signal r    : unsigned(63 downto 0);    -- remainder
signal qNeg : boolean;
signal rNeg : boolean;


signal dd_2     : unsigned(127 downto 0);   -- dividend
signal dr_2     : unsigned(63 downto 0);    -- divisor
signal q_2      : unsigned(63 downto 0);    -- qoutient
signal r_2      : unsigned(63 downto 0);    -- remainder
signal qNeg_2   : boolean;
signal rNeg_2   : boolean;


signal dd_3     : unsigned(127 downto 0);   -- dividend
signal dr_3     : unsigned(63 downto 0);    -- divisor
signal q_3      : unsigned(63 downto 0);    -- qoutient
signal r_3      : unsigned(63 downto 0);    -- remainder
signal qNeg_3   : boolean;
signal rNeg_3   : boolean;


signal dd_4     : unsigned(127 downto 0);   -- dividend
signal dr_4     : unsigned(63 downto 0);    -- divisor
signal q_4      : unsigned(63 downto 0);    -- qoutient
signal r_4      : unsigned(63 downto 0);    -- remainder
signal qNeg_4   : boolean;
signal rNeg_4   : boolean;


signal dd_5     : unsigned(127 downto 0);   -- dividend
signal dr_5     : unsigned(63 downto 0);    -- divisor
signal q_5      : unsigned(63 downto 0);    -- qoutient
signal r_5      : unsigned(63 downto 0);    -- remainder
signal qNeg_5   : boolean;
signal rNeg_5   : boolean;


signal dd_swap      : unsigned(127 downto 0);   -- dividend
signal dr_swap      : unsigned(63 downto 0);    -- divisor
signal q_swap       : unsigned(63 downto 0);    -- qoutient
signal r_swap       : unsigned(63 downto 0);    -- remainder
signal qNeg_swap    : boolean;
signal rNeg_swap    : boolean;


signal Exception        : boolean;
signal Exception_2      : boolean;
signal Exception_3      : boolean;
signal Exception_4      : boolean;
signal Exception_swap   : boolean;

signal Counter      : natural range 0 to 127;
signal Counter_Swap : natural range 0 to 127;


signal s_ready      : std_logic;
signal s_ready_2    : std_logic;
signal s_ready_3    : std_logic;
signal s_ready_4    : std_logic;
signal s_ready_5    : std_logic;
signal s_ready_swap : std_logic;



signal s_done       : boolean;
signal s_done_swap  : boolean;




begin
	
	
	
	process (division,remaind,funct,alu32,dividend,divisor)
	
	variable var_qNeg : boolean;
	variable var_rNeg : boolean;
		
	begin
		
		if division = '0' and remaind = '0' then
		
			dd <= (others => '0');
			dr <= (others => '0');
			
			qNeg <= FALSE;
			rNeg <= FALSE;
			
			s_alu32 <= '0';
			s_ready <= '0';
		
		else
		
			var_qNeg := FALSE;
			var_rNeg := FALSE;
		
		
			if alu32 = '1' then
			
				if funct(0) = '0' then
				
					if dividend(31) = '1' then
						var_qNeg := TRUE;
						var_rNeg := TRUE;
						dd(31 downto 0) <= unsigned(-signed(dividend(31 downto 0)));
					else
						dd(31 downto 0) <= unsigned(dividend(31 downto 0));
					end if;
					
					if divisor(31) = '1' then
						var_qNeg := not var_qNeg;
						dr(31 downto 0) <= unsigned(-signed(divisor(31 downto 0)));
					else
						dr(31 downto 0) <= unsigned(divisor(31 downto 0));
					end if;
				else
					dd(31 downto 0) <= unsigned(dividend(31 downto 0));
					dr(31 downto 0) <= unsigned(divisor(31 downto 0));
				end if;
				
				dd(127 downto 32) <= (others => '0');
				dr(63 downto 32) <= (others => '0');
			
			else
			
				if funct(0) = '0' then
					if dividend(63) = '1' then
						var_qNeg := TRUE;
						var_rNeg := TRUE;
						dd(63 downto 0) <= unsigned(-signed(dividend));
					else
						dd(63 downto 0) <= unsigned(dividend);
					end if;
					
					if divisor(63) = '1' then
						var_qNeg := not var_qNeg;
						dr <= unsigned(-signed(divisor));
					else
						dr <= unsigned(divisor);
					end if;
				else
					dd(63 downto 0) <= unsigned(dividend);
					dr <= unsigned(divisor);
				end if;
				
				dd(127 downto 64) <= (others => '0');
			
			end if;
			
			
			qNeg <= var_qNeg;
			rNeg <= var_rNeg;
			
			s_alu32 <= alu32;
			s_ready <= '1';
		
		end if;
		
	end process;
		
		
		
		
		
	process (CLK,nRST)
		
	begin
		
		if nRST = '0' then
		
			dd_2 <= (others => '0');
			dr_2 <= (others => '0');
			qNeg_2 <= FALSE;
			rNeg_2 <= FALSE;
		
			s_alu32_2 <= '0';
			s_ready_2 <= '0';
		
		elsif rising_edge(CLK) then
		
			dd_2 <= dd;
			dr_2 <= dr;
			qNeg_2 <= qNeg;
			rNeg_2 <= rNeg;
		
			s_alu32_2 <= s_alu32;
			s_ready_2 <= s_ready;
		
		end if;
		
	end process;
		
		
		
		
		
	process (dd_2,dr_2,s_alu32_2)
		
		
	begin
		
		
		if s_alu32_2 = '1' then
				
			if (dr_2=0) then  
				q(63 downto 0) <= (others=>'1');
				r(31 downto 0) <= dd_2(31 downto 0);
				r(63 downto 32) <= (others=>dd_2(31));
				Exception <= TRUE;
			elsif (dr_2>dd_2) then 
				q <= (others=>'0');
				r(31 downto 0) <= dd_2(31 downto 0);
				r(63 downto 32) <= (others=>dd_2(31));
				Exception <= TRUE;
			elsif (dr_2=dd_2(63 downto 0)) then
				q <= (0 => '1',others=>'0');
				r <= (others=>'0');
				Exception <= TRUE;
			else
				q <= (others=>'0');
				r <= (others=>'0');
				Exception <= FALSE;
			end if;
		
		else
		
			if (dr_2=0) then  
				q <= (others=>'1');
				r <= dd_2(63 downto 0);
				Exception <= TRUE;
			elsif (dr_2>dd_2) then 
				q <= (others=>'0');
				r <= dd_2(63 downto 0);
				Exception <= TRUE;
			elsif (dr_2=dd_2(63 downto 0)) then
				q <= (0 => '1',others=>'0');
				r <= (others=>'0');
				Exception <= TRUE;
			else				
				q <= (others=>'0');
				r <= (others=>'0');
				Exception <= FALSE;
			end if;
			
		end if;
	
	
	end process;
		
		
		
		
		
	process (CLK,nRST)
	
	variable var_maxcounter : natural range 0 to 127;
	
	begin
	
		if nRST = '0' then
		
			q_2 <= (others => '0'); 
			r_2 <= (others => '0');
			Exception_2 <= FALSE;
		
			dd_3 <= (others => '0');
			dr_3 <= (others => '0');
			qNeg_3 <= FALSE;
			rNeg_3 <= FALSE;
		
			s_alu32_3 <= '0';
			s_ready_3 <= '0';
		
		
		elsif rising_edge(CLK) then
		
			q_2 <= q;
			r_2 <= r;
			Exception_2 <= Exception;
		
			dd_3 <= dd_2;
			dr_3 <= dr_2;
			qNeg_3 <= qNeg_2;
			rNeg_3 <= rNeg_2;
		
			s_alu32_3 <= s_alu32_2;
			s_ready_3 <= s_ready_2;
		
		end if;
	
	
	
	end process;
		
		
		
		
		
	process (dd_3,dr_3,q_2,r_2,Exception_2,qNeg_3,rNeg_3,s_alu32_3,s_ready_3,
				dd_swap,dr_swap,q_swap,r_swap,Exception_swap,qNeg_swap,rNeg_swap,
				s_alu32_swap,s_ready_swap,Counter_Swap)
	
	variable var_dd : unsigned(127 downto 0);
	variable var_dr : unsigned(63 downto 0);
	variable var_alu32 : std_logic;
	variable var_counter : natural range 0 to 127;
	variable var_maxcounter : natural range 0 to 127;
	
	begin
	
		if s_ready_swap = '0' then
		
			var_dd := shift_left(dd_3,1);
			var_dr := dr_3;
			var_counter := 0;
			var_alu32 := s_alu32_3;
			
		else
		
			var_dd := shift_left(dd_swap,1);
			var_dr := dr_swap;
			var_counter := Counter_Swap;
			var_alu32 := s_alu32_swap;
		
		end if;
	
				
		if var_alu32 = '1' then
		
			if var_dd(63 downto 32) >= var_dr(31 downto 0) then
				var_dd(63 downto 32) := var_dd(63 downto 32) - var_dr(31 downto 0);
				var_dd(0) := '1';
			end if;
			
			var_maxcounter := 32;
		
		else
		
			if var_dd(127 downto 64) >= var_dr then
				var_dd(127 downto 64) := var_dd(127 downto 64) - var_dr;
				var_dd(0) := '1';
			end if;
			
			var_maxcounter := 64;
			
		end if;
		
		var_counter := var_counter + 1;
		
		Counter <= var_counter;	
	
		if s_ready_swap = '0' then
		
			q_3 <= q_2;
			r_3 <= r_2;
			
			Exception_3 <= Exception_2;
			
			dd_4 <= var_dd;
			dr_4 <= var_dr;
			
			qNeg_4 <= qNeg_3;
			rNeg_4 <= rNeg_3;
		
			s_alu32_4 <= s_alu32_3;
			s_ready_4 <= s_ready_3;
			
			s_done <= FALSE; 
		
		else
		
			q_3 <= q_swap;
			r_3 <= r_swap;
			
			Exception_3 <= Exception_swap;
			
			dd_4 <= var_dd;
			dr_4 <= var_dr;
			
			qNeg_4 <= qNeg_swap;
			rNeg_4 <= rNeg_swap;
		
			s_alu32_4 <= var_alu32;
			s_ready_4 <= s_ready_swap;
			
		
			if var_counter = var_maxcounter then
			
				s_done <= TRUE;
				
			else		
			
				s_done <= FALSE;
				
			end if;
		
		end if;
	
	
	end process;
		
		
		
		
		
	process (CLK,nRST)
	
	begin
	
		if nRST = '0' then
		
			Counter_Swap <= 0;
		
			q_4 <= (others => '0'); 
			r_4 <= (others => '0');
			Exception_4 <= FALSE;
		
			dd_5 <= (others => '0');
			qNeg_5 <= FALSE;
			rNeg_5 <= FALSE;
		
			s_alu32_5 <= '0';
			s_ready_5 <= '0';
			
			q_swap <= (others => '0');
			r_swap <= (others => '0');
			Exception_swap <= FALSE;
		
			dd_swap <= (others => '0');
			dr_swap <= (others => '0');
			qNeg_swap <= FALSE;
			rNeg_swap <= FALSE;
		
			s_alu32_swap <= '0';
			s_ready_swap <= '0';
		
		
		elsif rising_edge(CLK) then
		
			Counter_Swap <= Counter;
		
			if s_ready_4 = '0' or s_done then
			
				q_4 <= q_3;
				r_4 <= r_3;
				Exception_4 <= Exception_3;
			
				dd_5 <= dd_4;
				qNeg_5 <= qNeg_4;
				rNeg_5 <= rNeg_4;
			
				s_alu32_5 <= s_alu32_4;
				s_ready_5 <= s_ready_4;
			
				q_swap <= (others => '0');
				r_swap <= (others => '0');
				Exception_swap <= FALSE;
			
				dd_swap <= (others => '0');
				dr_swap <= (others => '0');
				qNeg_swap <= FALSE;
				rNeg_swap <= FALSE;
			
				s_alu32_swap <= '0';
				s_ready_swap <= '0';
			
			else
			
				q_4 <= (others => '0');
				r_4 <= (others => '0');
				Exception_4 <= FALSE;
			
				dd_5 <= (others => '0');
				qNeg_5 <= FALSE;
				rNeg_5 <= FALSE;
			
				s_alu32_5 <= '0';
				s_ready_5 <= '0';
			
				q_swap <= q_3;
				r_swap <= r_3;
				Exception_swap <= Exception_3;
			
				dd_swap <= dd_4;
				dr_swap <= dr_4;
				qNeg_swap <= qNeg_4;
				rNeg_swap <= rNeg_4;
			
				s_alu32_swap <= s_alu32_4;
				s_ready_swap <= s_ready_4;
			
			end if;
		
		
		end if;
	
	
	
	end process;
		
		
		
		
	process (q_4,r_4,Exception_4,dd_5,qNeg_5,rNeg_5,s_alu32_5,s_ready_5)
	
	variable var_q : unsigned(63 downto 0);
	variable var_r : unsigned(63 downto 0);
	
	begin
	
		if Exception_4 then
			if s_alu32_5 = '1' then
				var_q := x"00000000" & q_4(31 downto 0);
				var_r := x"00000000" & r_4(31 downto 0);
			else
				var_q := q_4;
				var_r := r_4;
			end if;
		else
			if s_alu32_5 = '1' then
				var_q := x"00000000" & dd_5(31 downto 0);
				var_r := x"00000000" & dd_5(63 downto 32);
			else
				var_q(63 downto 0) := dd_5(63 downto 0);
				var_r(63 downto 0) := dd_5(127 downto 64);
			end if;
		end if;
	
		if qNeg_5 and (not Exception_4) then
			var_q := "0" - var_q;
		end if;
		
		if rNeg_5 then
			var_r := "0" - var_r;
		end if;
	
		if s_alu32_5 = '1' then
			quotient(31 downto 0)  <= std_logic_vector(var_q(31 downto 0));
			remainder(31 downto 0) <= std_logic_vector(var_r(31 downto 0));
			quotient(63 downto 32) 	<= (others => var_q(31));
			remainder(63 downto 32) <= (others => var_r(31));
		else
			quotient  <= std_logic_vector(var_q);
			remainder <= std_logic_vector(var_r);
		end if;
		
		ready <= s_ready_5;
		
	
	end process;
	
	
end;
