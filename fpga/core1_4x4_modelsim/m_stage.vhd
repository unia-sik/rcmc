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
		global_stall_out : out std_logic;
		nbi              : in  P_PORT_BUFFER;
		nbo              : out P_PORT_BUFFER;
		nbr              : out REQUEST_PORT;
		nbsf             : in  std_logic
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
		variable q  : std_logic_vector(63 downto 0);
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
			q := data;
		elsif byteen = "00001111" or byteen = "11110000" then
			q := we;
		elsif byteen = "00000011" or byteen = "00001100" or byteen = "00110000" or byteen = "11000000" then
			q := he;
		else
			q := be;
		end if;

		return q;
	end function rdataout;

	function wdataout(
		byteen : std_logic_vector(7 downto 0);
		data   : std_logic_vector(63 downto 0)
		) return std_logic_vector is
		variable q : std_logic_vector(63 downto 0);
	begin
		q := data;
		case byteen is
			when "00000001" =>
				q(7 downto 0) := data(7 downto 0);
			when "00000010" =>
				q(15 downto 8) := data(7 downto 0);
			when "00000100" =>
				q(23 downto 16) := data(7 downto 0);
			when "00001000" =>
				q(31 downto 24) := data(7 downto 0);
			when "00010000" =>
				q(39 downto 32) := data(7 downto 0);
			when "00100000" =>
				q(47 downto 40) := data(7 downto 0);
			when "01000000" =>
				q(55 downto 48) := data(7 downto 0);
			when "10000000" =>
				q(63 downto 56) := data(7 downto 0);
			when "00000011" =>
				q(15 downto 0) := data(15 downto 0);
			when "00001100" =>
				q(31 downto 16) := data(15 downto 0);
			when "00110000" =>
				q(47 downto 32) := data(15 downto 0);
			when "11000000" =>
				q(63 downto 48) := data(15 downto 0);
			when "00001111" =>
				q(31 downto 0) := data(31 downto 0);
			when "11110000" =>
				q(63 downto 32) := data(31 downto 0);
			when others => null;
		end case;

		return q;
	end function wdataout;

	signal r, rin : memory_reg_type;

	signal stall_noc_request      : std_logic;
	signal stall_noc_request_over : std_logic;

begin
	comb : process(d, a, dco, r, nbi, nbsf, stall_noc_request, stall_noc_request_over)
		variable v       : memory_reg_type;
		variable v_stall : std_logic;
	begin
		v := r;

		v.pc := d.e.pc;

		-- compute request
		if d.e.fgmop = '1' and nbi.DataAvailable = '0' then
			v.res := d.e.res;
			if d.e.lsfunc = FUNC_FGMP_RECV then
				-- CoreBuffer expects addr as XY
				-- CPU expects addr as YX
				nbr.Address(Address_Length_Y + Address_Length_X - 1 downto Address_Length_Y) <=  d.e.res(Address_Length_X - 1 downto 0);
				nbr.Address(Address_Length_Y - 1 downto 0) <=  d.e.res(Address_Length_X + Address_Length_Y - 1 downto Address_Length_X);
				nbr.Request         <= '1';
				nbr.SpecificRequest <= '1';
				nbr.ProbeRequest    <= '0';
				stall_noc_request   <= '1';
			elsif d.e.lsfunc = FUNC_FGMP_PROBE then
				nbr.Address(Address_Length_Y + Address_Length_X - 1 downto Address_Length_Y) <=  d.e.res(Address_Length_X - 1 downto 0);
				nbr.Address(Address_Length_Y - 1 downto 0) <=  d.e.res(Address_Length_X + Address_Length_Y - 1 downto Address_Length_X);
				nbr.Request         <= '1';
				nbr.SpecificRequest <= '1';
				nbr.ProbeRequest    <= '1';
				stall_noc_request   <= '1';
			elsif d.e.lsfunc = FUNC_FGMP_WAIT then
				nbr.Address         <= (others => '0');
				nbr.Request         <= '1';
				nbr.SpecificRequest <= '0';
				nbr.ProbeRequest    <= '1';
				stall_noc_request   <= '1';
			elsif d.e.lsfunc = FUNC_FGMP_ANY then
				nbr.Address         <= (others => '0');
				nbr.Request         <= '1';
				nbr.SpecificRequest <= '0';
				nbr.ProbeRequest    <= '1';
				stall_noc_request   <= '1';
			else
				nbr.Address         <= (others => '0');
				nbr.Request         <= '0';
				nbr.SpecificRequest <= '0';
				nbr.ProbeRequest    <= '0';
				stall_noc_request   <= '0';
			end if;
		else
			nbr.Address         <= (others => '0');
			nbr.Request         <= '0';
			nbr.SpecificRequest <= '0';
			nbr.ProbeRequest    <= '0';
			stall_noc_request   <= '0';
		end if;

		-- stall?
		if a.e.fgmop = '1' and a.e.lsfunc = FUNC_FGMP_SEND and nbsf = '1' then
			v_stall := '1';             -- going to send but send buffer full
		elsif d.e.fgmop = '1' and stall_noc_request = '1' then
			case d.e.lsfunc is
				when FUNC_FGMP_RECV 	=> v_stall := not (nbi.DataAvailable and stall_noc_request_over); -- stall as long as there is no data avail but wait atleast one tick
				when FUNC_FGMP_PROBE	=> v_stall := not stall_noc_request_over; -- wait atleast one tick
				when FUNC_FGMP_WAIT 	=> v_stall := not (nbi.DataAvailable and stall_noc_request_over); -- stall as long as there is no data avail but wait atleast one tick
				when FUNC_FGMP_ANY		=> v_stall := not stall_noc_request_over; -- wait atleast one tick
				when others         	=> v_stall := '0';
			end case;
		else
			v_stall := '0';
		end if;

		v.exception := d.e.exception;

		dci.addr   <= d.e.res;
		dci.wdata  <= wdataout(d.e.dbyteen, d.e.rs2);
		dci.we     <= d.e.dwe and not v.exception.valid and not d.w.exception;
		dci.byteen <= d.e.dbyteen;

		if d.e.fgmop = '1' and d.e.lsfunc = FUNC_FGMP_SEND then
			nbo.DataAvailable <= '1';
			nbo.Data          <= d.e.rs2;
			--nbo.Address       <= d.e.res((Address_Length_X + Address_Length_Y) - 1 DOWNTO 0);
			nbo.Address(Address_Length_Y + Address_Length_X - 1 downto Address_Length_Y) <=  d.e.res(Address_Length_X - 1 downto 0);
			nbo.Address(Address_Length_Y - 1 downto 0) <=  d.e.res(Address_Length_X + Address_Length_Y - 1 downto Address_Length_X);
		else
			nbo.DataAvailable <= '0';
			nbo.Data          <= (others => '0');
			nbo.Address       <= (others => '0');
		end if;

		-- result
		v.res := (others => '0');

		v.we    := d.e.we and not v.exception.valid and not d.w.exception;

		if v_stall = '1' then
			v.we := '0';
		end if;

		-- todo: ignore while stalling?
		if d.e.fgmop = '1' then
			case d.e.lsfunc is
				when FUNC_FGMP_RECV  => if nbi.DataAvailable = '1' then v.res := nbi.Data; end if; -- return data
				when FUNC_FGMP_PROBE => v.res(0) := nbi.DataAvailable; -- return if data avail
				when FUNC_FGMP_WAIT  => v.res((Address_Length_X + Address_Length_Y) - 1 DOWNTO 0) := nbi.Address;
				when FUNC_FGMP_CONG  => v.res(0) := nbsf;
				when FUNC_FGMP_ANY   => if nbi.DataAvailable = '1' then
											v.res(Address_Length_Y + Address_Length_X - 1 downto Address_Length_Y) :=  nbi.Address(Address_Length_X - 1 downto 0);
											v.res(Address_Length_Y - 1 downto 0) :=  nbi.Address(Address_Length_X + Address_Length_Y - 1 downto Address_Length_X);
										else
											v.res := (others => '1');
										end if;
				when others          => v.res := (others => '0');
			end case;
		elsif d.e.dre = '1' then
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