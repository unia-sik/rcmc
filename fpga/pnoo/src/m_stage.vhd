library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.constants.all;
use work.libeu.all;
use work.libproc.all;
USE work.LibNode.ALL;

entity memory_stage is
	port(
		clk              : in  std_logic;
		rst_n            : in  std_logic;
		dco              : in  dcache_out_type;
		dci              : out dcache_in_type;
		d                : in  memory_in_type;
		a                : in  memory_in_type; -- input for unregistered signals
		q                : out memory_out_type;
		y                : out memory_out_type;
		global_stall     : in  std_logic;
		global_stall_out : out std_logic
	);

end entity memory_stage;

architecture rtl of memory_stage is
	alias memory_reg_type is memory_out_type;

	function rdataout(
		sign   : std_logic;
		byteen : std_logic_vector(7 downto 0);
		data   : std_logic_vector(63 downto 0)
		) return std_logic_vector is
		variable b  : std_logic_vector(7 downto 0); -- byte
		variable h  : std_logic_vector(15 downto 0); -- halfword
		variable w  : std_logic_vector(31 downto 0); -- word
		variable be : std_logic_vector(63 downto 0); -- byte extended
		variable he : std_logic_vector(63 downto 0); -- halfword extended
		variable we : std_logic_vector(63 downto 0); -- word extended
		variable res  : std_logic_vector(63 downto 0);
	begin
		b := (others => '0');
		h := (others => '0');
		w := (others => '0');
		case byteen is
			when "00000001" =>
				b := data(7 downto 0);
			when "00000010" =>
				b := data(15 downto 8);
			when "00000100" =>
				b := data(23 downto 16);
			when "00001000" =>
				b := data(31 downto 24);
			when "00010000" =>
				b := data(39 downto 32);
			when "00100000" =>
				b := data(47 downto 40);
			when "01000000" =>
				b := data(55 downto 48);
			when "10000000" =>
				b := data(63 downto 56);

			when "00000011" =>
				h := data(15 downto 0);
			when "00001100" =>
				h := data(31 downto 16);
			when "00110000" =>
				h := data(47 downto 32);
			when "11000000" =>
				h := data(63 downto 48);

			when "00001111" =>
				w := data(31 downto 0);
			when "11110000" =>
				w := data(63 downto 32);
			when others => null;
		end case;

		if sign = '1' then
			be := std_logic_vector(resize(unsigned(b), 64));
			he := std_logic_vector(resize(unsigned(h), 64));
			we := std_logic_vector(resize(unsigned(w), 64));
		else
			be := std_logic_vector(resize(signed(b), 64));
			he := std_logic_vector(resize(signed(h), 64));
			we := std_logic_vector(resize(signed(w), 64));
		end if;

		if byteen = "11111111" then
			res := data;
		elsif byteen = "00001111" or byteen = "11110000" then
			res := we;
		elsif byteen = "00000011" or byteen = "00001100" or byteen = "00110000" or byteen = "11000000" then
			res := he;
		else
			res := be;
		end if;

		return res;
	end function rdataout;

	function wdataout(
		byteen : std_logic_vector(7 downto 0);
		data   : std_logic_vector(63 downto 0)
		) return std_logic_vector is
		variable res : std_logic_vector(63 downto 0);
	begin
		res := data;
		case byteen is
			when "00000001" =>
				res(7 downto 0) := data(7 downto 0);
			when "00000010" =>
				res(15 downto 8) := data(7 downto 0);
			when "00000100" =>
				res(23 downto 16) := data(7 downto 0);
			when "00001000" =>
				res(31 downto 24) := data(7 downto 0);
			when "00010000" =>
				res(39 downto 32) := data(7 downto 0);
			when "00100000" =>
				res(47 downto 40) := data(7 downto 0);
			when "01000000" =>
				res(55 downto 48) := data(7 downto 0);
			when "10000000" =>
				res(63 downto 56) := data(7 downto 0);
			when "00000011" =>
				res(15 downto 0) := data(15 downto 0);
			when "00001100" =>
				res(31 downto 16) := data(15 downto 0);
			when "00110000" =>
				res(47 downto 32) := data(15 downto 0);
			when "11000000" =>
				res(63 downto 48) := data(15 downto 0);
			when "00001111" =>
				res(31 downto 0) := data(31 downto 0);
			when "11110000" =>
				res(63 downto 32) := data(31 downto 0);
			when others => null;
		end case;

		return res;
	end function wdataout;

	signal r, rin : memory_reg_type;

	signal stall_noc_request      : std_logic;
	signal stall_noc_request_over : std_logic;

begin
	comb : process(d, a, dco, r, stall_noc_request, stall_noc_request_over)
		variable v       : memory_reg_type;
		variable v_stall : std_logic;
	begin
		v := r;

		v.pc := d.e.pc;

		stall_noc_request   <= '0';

		v_stall := '0';

		v.exception := d.e.exception;

		dci.addr   <= d.e.res;
		dci.wdata  <= wdataout(d.e.dbyteen, d.e.rs2);
		dci.we     <= d.e.dwe and not v.exception.valid and not d.w.exception;
		dci.byteen <= d.e.dbyteen;

		-- result
		v.res := (others => '0');

		v.we := d.e.we and not v.exception.valid and not d.w.exception;

		if v_stall = '1' then
			v.we := '0';
		end if;

		-- todo: ignore while stalling?
		if d.e.dre = '1' then
			v.res := rdataout(d.e.lsfunc(2), d.e.dbyteen, dco.rdata);
		else
			v.res := d.e.res;
		end if;

		v.waddr := d.e.waddr;

		v.csraddr := d.e.csraddr;
		v.csrval  := d.e.csrval;
		v.csrwe   := d.e.csrwe;

		-- we are jumping or something down the pipe is throwing
		if r.exception.valid = '1'      -- we are currently throwing (nop next input)
		or d.w.exception = '1'          -- writeback stage
		then
			v.we              := '0';
			v.csrwe           := '0';
			v.exception.valid := '0';
			v_stall           := '0';
		end if;

		-- we are throwing
		if v.exception.valid = '1' then
			v.we            := '0';
			v.csrwe         := '0';
			v.exception.epc := v.pc;
			v_stall         := '0';
		end if;
		rin <= v;

		global_stall_out <= v_stall;

		y <= v;

		v := r;

		q <= v;
	end process;

	reg : process(clk, rst_n)           -- sequential process
	begin
		if rst_n = '0' then
			r.res             <= (others => '0');
			r.waddr           <= (others => '0');
			r.we              <= '0';
			r.csrwe           <= '0';
			r.exception.epc   <= (others => '0');
			r.exception.cause <= (others => '0');
			r.exception.valid <= '0';

			stall_noc_request_over <= '0';
		elsif rising_edge(clk) then
			if global_stall = '0' then
				r <= rin;
			end if;

			stall_noc_request_over <= stall_noc_request;
		end if;
	end process;

end architecture;