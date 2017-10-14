library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;

library altera_mf;
use altera_mf.all;

entity regfile is
	port(
		clk          : in  std_logic;
		rst_n        : in  std_logic;
		rfri         : in  regfile_read_in_type;
		rfwi         : in  regfile_write_in_type;
		rfo          : out regfile_out_type;
		global_stall : in  std_logic
	);

end regfile;

architecture rtl of regfile is
	COMPONENT regfile_ram
		PORT(
			data      : IN  STD_LOGIC_VECTOR(63 DOWNTO 0);
			rdaddress : IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
			rdclock   : IN  STD_LOGIC;
			rdclocken : IN  STD_LOGIC := '1';
			wraddress : IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
			wrclock   : IN  STD_LOGIC := '1';
			wrclocken : IN  STD_LOGIC := '1';
			wren      : IN  STD_LOGIC := '0';
			q         : OUT STD_LOGIC_VECTOR(63 DOWNTO 0)
		);
	END COMPONENT;

	signal raddr1_reg : std_logic_vector(4 downto 0);
	signal raddr2_reg : std_logic_vector(4 downto 0);
	signal waddr_reg  : std_logic_vector(4 downto 0);
	signal we_reg     : std_logic;
	signal wdata_reg  : std_logic_vector(63 downto 0);
	signal rdata1_reg : std_logic_vector(63 downto 0);
	signal rdata2_reg : std_logic_vector(63 downto 0);

	constant zero : std_logic_vector(4 downto 0) := (others => '0');

begin
	REG1 : regfile_ram
		port map(
			data      => rfwi.wdata,
			rdaddress => rfri.raddr1,
			rdclock   => clk,
			rdclocken => rfri.rclke,
			wraddress => rfwi.waddr,
			wrclock   => clk,
			wrclocken => rfwi.wclke,
			wren      => rfwi.we,
			q         => rdata1_reg
		);

	REG2 : regfile_ram
		port map(
			data      => rfwi.wdata,
			rdaddress => rfri.raddr2,
			rdclock   => clk,
			rdclocken => rfri.rclke,
			wraddress => rfwi.waddr,
			wrclock   => clk,
			wrclocken => rfwi.wclke,
			wren      => rfwi.we,
			q         => rdata2_reg
		);

	process(clk, rst_n)
	begin
		if rst_n = '0' then
			raddr1_reg <= (others => '0');
			raddr2_reg <= (others => '0');
			waddr_reg  <= (others => '0');
			wdata_reg  <= (others => '0');
			we_reg     <= '0';
		elsif (rising_edge(clk)) then
			if global_stall = '0' then
				if (rfri.rclke = '1') then
					raddr1_reg <= rfri.raddr1;
					raddr2_reg <= rfri.raddr2;
				end if;
				if (rfwi.wclke = '1') then
					waddr_reg <= rfwi.waddr;
					wdata_reg <= rfwi.wdata;
					we_reg    <= rfwi.we;
				end if;
			end if;
		end if;
	end process;

	process(raddr1_reg, raddr2_reg, waddr_reg, we_reg, wdata_reg, rdata1_reg, rdata2_reg)
	begin
		if we_reg = '1' and waddr_reg = raddr1_reg then
			rfo.data1 <= wdata_reg;
		else
			rfo.data1 <= rdata1_reg;
		end if;

		if we_reg = '1' and waddr_reg = raddr2_reg then
			rfo.data2 <= wdata_reg;
		else
			rfo.data2 <= rdata2_reg;
		end if;
	end process;

end rtl;