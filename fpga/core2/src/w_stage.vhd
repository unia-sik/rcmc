library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;

entity writeback_stage is
	port(
		clk          : in  std_logic;
		rst_n        : in  std_logic;
		rfwi         : out regfile_write_in_type;
		fprwi        : out fregfile_write_in_type;
		csrwi        : out csrfile_write_in_type;
		d            : in  writeback_in_type;
		a            : in  writeback_in_type;
		q            : out writeback_out_type;
		y            : out writeback_out_type;
		global_stall : in  std_logic
	);

end entity writeback_stage;

architecture rtl of writeback_stage is
	signal r, rin : writeback_out_type;

begin
	comb : process(a, d, r)
		variable v : writeback_out_type;
	begin
		v          := r;
		-- writeback of registers
		rfwi.wclke <= '1';
		rfwi.waddr <= a.m.waddr;
		rfwi.wdata <= a.m.res;
		rfwi.we    <= a.m.we;

		-- writeback of fpu registers
		fprwi.wclke <= '1';
		fprwi.waddr <= a.m.fpwaddr;
		fprwi.wdata <= a.m.fpres;
		fprwi.we    <= a.m.fpwe;
		fprwi.dword <= a.m.dword;

		-- writeback of fcsr registers
		csrwi.fflags <= a.m.fflags;
        csrwi.fwe 	 <= a.m.csrfwe;

		-- writeback of csr registers
		csrwi.wclke <= '1';
		csrwi.waddr <= a.m.csraddr;
		csrwi.wdata <= a.m.csrval;
		csrwi.we    <= a.m.csrwe;

		if d.m.exception.valid = '1' then
			csrwi.cause <= d.m.exception.cause;
			csrwi.epc   <= d.m.exception.epc;
			csrwi.ewe   <= '1';
		else
			csrwi.cause <= (others => '-');
			csrwi.epc   <= (others => '-');
			csrwi.ewe   <= '0';
		end if;

		-- is there an exception?
		-- TODO: csr/regfile exceptions
		v.exception := d.m.exception.valid;

		rin <= v;
		q   <= r;
		y   <= v;
	end process;

	reg : process(clk, rst_n)           -- sequential process
	begin
		if rst_n = '0' then
			r.exception <= '0';
		elsif rising_edge(clk) then
			if global_stall = '0' then
				r <= rin;
			end if;
		end if;
	end process;
end architecture;
