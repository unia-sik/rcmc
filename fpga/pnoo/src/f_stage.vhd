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

begin
	comb : process(a, d, r, ico, csro)  -- combinational process
		variable v : fetch_reg_type;
	begin
		v    := r;
		v.pc := r.pc;

		if d.w.exception = '1' then
			v.pc := csro.evec;
		elsif d.e.jump = '0' then
			v.pc := r.pc4;
		else
			v.pc := d.e.pcjump;
		end if;

		v.pc4 := std_logic_vector(unsigned(v.pc) + 4);

		ici.addr <= v.pc;
		v.inst   := ico.data;

		-- ignore all changes
		if d.d.stall_m = '1' or a.d.stall_csr = '1' then
			v := r;
		end if;

		v.exception.cause := EXC_INST_ADDR_MISALIGNED;
		v.exception.epc   := v.pc;

		if v.pc(1 downto 0) = "00" then
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
			v.exception.valid := '0';
		end if;

		-- we are throwing
		if v.exception.valid = '1' then
			v.inst := x"00000013";
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
			r.inst            <= x"00000013";
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