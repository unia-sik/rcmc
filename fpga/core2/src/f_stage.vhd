library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libproc.all;
use work.libeu.all;

entity fetch_stage is
	port(
		clk          : in  std_logic;
		rst_n        : in  std_logic;
		csro         : in  csrfile_out_type;
		ico          : in  icache_out_type;
		ici          : out icache_in_type;
		mcco         : in  mcodecache_out_type;
		mcci         : out mcodecache_in_type;
		d            : in  fetch_in_type;
		a            : in  fetch_in_type;
		q            : out fetch_out_type;
		y            : out fetch_out_type;
		global_stall : in  std_logic
	);

end entity fetch_stage;

architecture rtl of fetch_stage is
	alias fetch_reg_type is fetch_out_type;

	signal r, rin : fetch_reg_type;
	
	signal test_fetch : std_logic_vector(63 downto 0);
	signal test_fetch2 : std_logic;
	signal test_fetch2d : std_logic;
	signal test_fetch2m : std_logic;
	signal test_fetch3 : std_logic;
	signal test_fetch4 : std_logic;
	signal test_fetch5 : std_logic;
	signal test_fetch5a : std_logic;
	signal test_fetch6 : std_logic_vector(63 downto 0);
	signal test_fetch7 : std_logic;
	signal test_fetch8 : std_logic;
	signal test_fetch9 : std_logic;
	signal test_fetch10 : std_logic;
	signal test_fetch10a : std_logic;
	signal test_fetch11 : std_logic_vector(63 downto 0);
	
	

begin
	comb : process(a, d, r, ico, mcco, csro)  -- combinational process
		variable v : fetch_reg_type;
	begin
		v    := r;
		v.pc := r.pc;
		v.mcpc := r.mcpc;
		
		test_fetch2 <= a.d.mcode;
		test_fetch2d <= d.d.mcode;
		test_fetch2m <= a.m.mcode;
		test_fetch3 <= d.w.exception;
		test_fetch4 <= d.e.jump;
		test_fetch5 <= d.d.stall_m;
		test_fetch5a <= a.d.stall_m;
		test_fetch7 <= d.d.exception.valid;
		test_fetch8 <= d.e.exception.valid;
		test_fetch9 <= d.m.exception.valid;
		
		test_fetch10 <= d.d.mcodefinish;
		test_fetch10a <= a.d.mcodefinish;

		if d.w.exception = '1' then
			v.pc := csro.evec;
		elsif a.d.mcodefinish = '1' then
			v.pc := r.pc4;
		elsif d.d.mcode = '1' then
			v.pc := r.pc;
		elsif d.e.jump = '0' then
			v.pc := r.pc4;
		else
			v.pc := d.e.pcjump;
		end if;
		
		test_fetch11 <= v.pc;

		v.pc4 := std_logic_vector(unsigned(v.pc) + 4);
		
		if d.d.mcode = '1' then
			v.mcpc := r.mcpc4;
		else
			v.mcpc := x"0000000000000000";
		end if;
		
		v.mcpc4 := std_logic_vector(unsigned(v.mcpc) + 4);

		ici.addr <= v.pc;
		v.inst      := ico.data;
		
		mcci.addr <= v.mcpc;
		v.mcodeinst := mcco.data;

		-- ignore all changes
		if a.d.mcodefinish = '1' and d.d.stall_m = '0' then
			
		elsif (d.d.mcode = '0' and d.d.stall_m = '1') or a.d.stall_csr = '1' or (d.d.mcode = '1' and a.d.stall_m = '1') then
			v := r;
		end if;
		
		test_fetch <= v.mcpc;
		test_fetch6 <=  v.pc;

		v.exception.cause := EXC_INST_ADDR_MISALIGNED;
		v.exception.epc   := v.pc;

		if v.pc(1 downto 0) = "00" and v.mcpc(1 downto 0) = "00" then
			v.exception.valid := '0';
		else
			v.exception.valid := '1';
		end if;

		-- something up the pipe is throwing or we are jumping
		if r.exception.valid = '1' -- we are currently throwing (nop next input)
		or d.d.exception.valid = '1'    -- decode stage
		or d.e.exception.valid = '1'    -- execute stage
		or d.m.exception.valid = '1'    -- memory stage
		then
			v.inst            := x"00000013";
			v.mcodeinst       := (0 => x"00000013",1 => x"00000013",2 => x"00000013",3 => x"00000013");
			v.exception.valid := '0';
		end if;

		-- we are throwing
		if v.exception.valid = '1' then
			v.inst := x"00000013";
			v.mcodeinst       := (0 => x"00000013",1 => x"00000013",2 => x"00000013",3 => x"00000013");
		end if;

		rin <= v;

		q <= r;

		y <= v;
	end process;

	reg : process(clk, rst_n)           -- sequential process
	begin
		if rst_n = '0' then
			r.pc              <= x"0000000000000000";
			r.pc4             <= x"0000000000000000";
			r.mcpc            <= x"0000000000000000";
			r.mcpc4           <= x"0000000000000000";
			r.inst            <= x"00000013";
			r.mcodeinst       <= (0 => x"00000013",1 => x"00000013",2 => x"00000013",3 => x"00000013");
			r.exception.epc   <= (others => '0');
			r.exception.cause <= (others => '0');
			r.exception.valid <= '0';
		elsif rising_edge(clk) then
			if global_stall = '0' then
				r <= rin;
			end if;
		end if;
	end process;

end architecture;
