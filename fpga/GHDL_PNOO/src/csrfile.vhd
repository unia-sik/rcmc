library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;

entity csrfile is
	generic(
		id    : integer;
		count : integer;
		nocdim : std_logic_vector(63 downto 0)
	);
	port(
		clk          : in  std_logic;
		rst_n        : in  std_logic;
		csrri        : in  csrfile_read_in_type;
		csrwi        : in  csrfile_write_in_type;
		csro         : out csrfile_out_type;
		global_stall : in  std_logic
	);

end csrfile;

architecture rtl of csrfile is
	-- in order, CSRs taken from the 2012 draft
	signal status : std_logic_vector(31 downto 0);
	signal epc    : std_logic_vector(63 downto 0);
	-- signal badvaddr   : std_logic_vector(63 downto 0);
	signal evec   : std_logic_vector(63 downto 0);
	-- signal count		: std_logic_vector(31 downto 0); -- maybe 64 bit?
	-- signal compare		: std_logic_vector(31 downto 0); -- maybe 64 bit?
	signal cause  : std_logic_vector(31 downto 0);
	-- signal ptbr			: std_logic_vector(...)
	-- 8: unused
	-- 9: unused
	--10: unused
	--11: unused
	--12: k0 - scratch register for exception handlers
	--13: k1 - scratch register for exception handlers
	--14: unused
	--15: unused
	-- signal tohost		: std_logic_vector(63 downto 0);
	-- signal fromhost	: std_logic_vector(63 downto 0);
	--18-31: unused
	signal cycle  : std_logic_vector(63 downto 0);
	-- time is cycle
	-- instret is cycle too for now

	signal read_address : std_logic_vector(11 downto 0);
begin
	process(clk, rst_n)
	begin
		if rst_n = '0' then
			status       <= (others => '0'); -- todo: real status
			epc          <= (others => '0');
			evec         <= (others => '0');
			cause        <= (others => '0');
			cycle        <= x"FFFFFFFFFFFFFFFF";
			read_address <= (others => '0');
		elsif (rising_edge(clk)) then
			if global_stall = '0' then
				-- increase cycle
				cycle <= std_logic_vector(unsigned(cycle) + 1);

				if csrri.rclke = '1' then
					read_address <= csrri.raddr;
				end if;

				csro.status <= status;
				csro.epc    <= epc;
				csro.evec   <= evec;
				csro.cause  <= cause;

				-- write
				if csrwi.we = '1' then
					case csrwi.waddr is
						when CSR_STATUS => status <= csrwi.wdata(31 downto 0);
						when CSR_EPC    => epc <= csrwi.wdata;
						when CSR_EVEC   => evec <= csrwi.wdata;
						when others     => null;
					end case;
				end if;

				-- direct write
				if csrwi.ewe = '1' then
					epc   <= csrwi.epc;
					cause <= "000000000000000000000000000" & csrwi.cause;
				end if;
			end if;
		end if;
	end process;

	process(read_address, status, epc, evec, cause, cycle)
	begin
		case read_address is
			when CSR_STATUS =>
				csro.rdata(63 downto 32) <= (others => '0');
				csro.rdata(31 downto 0)  <= status;
			when CSR_HARTID =>
				csro.rdata <= std_logic_vector(to_unsigned(id, csro.rdata'length));
			when CSR_EPC =>
				csro.rdata <= epc;
			when CSR_EVEC =>
				csro.rdata <= evec;
			when CSR_CAUSE =>
				csro.rdata(63 downto 32) <= (others => '0');
				csro.rdata(31 downto 0)  <= cause;
			when CSR_CYCLE =>
				csro.rdata <= cycle;
			when CSR_CYCLEH =>
				csro.rdata(63 downto 32) <= (others => '0');
				csro.rdata(31 downto 0)  <= cycle(63 downto 32);
			when CSR_TIME =>
				csro.rdata <= cycle;
			when CSR_TIMEH =>
				csro.rdata(63 downto 32) <= (others => '0');
				csro.rdata(31 downto 0)  <= cycle(63 downto 32);
			when CSR_INSTRET =>
				csro.rdata <= cycle;
			when CSR_INSTRETH =>
				csro.rdata(63 downto 32) <= (others => '0');
				csro.rdata(31 downto 0)  <= cycle(63 downto 32);
			when CSR_FGMP_MAXCID =>
				csro.rdata <= std_logic_vector(to_unsigned(count, csro.rdata'length));
			when CSR_FGMP_CID =>
				csro.rdata <= std_logic_vector(to_unsigned(id, csro.rdata'length));
			when CSR_FGMP_NOCDIM =>
				csro.rdata <= nocdim;
			when others =>
				csro.rdata(63 downto 0) <= (others => '0');
		end case;
	end process;
end rtl;
