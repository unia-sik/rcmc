library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;

entity fregfile is
	port(
		clk          : in  std_logic;
		rst_n        : in  std_logic;
		fprri         : in  fregfile_read_in_type;
		fprwi         : in  fregfile_write_in_type;
		fpro          : out fregfile_out_type;
		global_stall : in  std_logic
	);

end fregfile;

architecture rtl of fregfile is
	COMPONENT fregfile_ram
		PORT(
			data      : IN  STD_LOGIC_VECTOR(63 DOWNTO 0);
			rdaddress : IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
			rdclock   : IN  STD_LOGIC;
			rdclocken : IN  STD_LOGIC := '1';
			wraddress : IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
			wrclock   : IN  STD_LOGIC := '1';
			wrclocken : IN  STD_LOGIC := '1';
			wren      : IN  STD_LOGIC := '0';
		  dword     : IN  STD_LOGIC := '0';
			q         : OUT STD_LOGIC_VECTOR(63 DOWNTO 0);
			freg0	: out std_logic_vector(63 downto 0);
			freg1	: out std_logic_vector(63 downto 0);
			freg2	: out std_logic_vector(63 downto 0);
			freg3	: out std_logic_vector(63 downto 0);
			freg4	: out std_logic_vector(63 downto 0);
			freg5	: out std_logic_vector(63 downto 0);
			freg6	: out std_logic_vector(63 downto 0);
			freg7	: out std_logic_vector(63 downto 0);
			freg8	: out std_logic_vector(63 downto 0);
			freg9	: out std_logic_vector(63 downto 0);
			freg10	: out std_logic_vector(63 downto 0);
			freg11	: out std_logic_vector(63 downto 0);
			freg12	: out std_logic_vector(63 downto 0);
			freg13	: out std_logic_vector(63 downto 0);
			freg14	: out std_logic_vector(63 downto 0);
			freg15	: out std_logic_vector(63 downto 0);
			freg16	: out std_logic_vector(63 downto 0);
			freg17	: out std_logic_vector(63 downto 0);
			freg18	: out std_logic_vector(63 downto 0);
			freg19	: out std_logic_vector(63 downto 0);
			freg20	: out std_logic_vector(63 downto 0);
			freg21	: out std_logic_vector(63 downto 0);
			freg22	: out std_logic_vector(63 downto 0);
			freg23	: out std_logic_vector(63 downto 0);
			freg24	: out std_logic_vector(63 downto 0);
			freg25	: out std_logic_vector(63 downto 0);
			freg26	: out std_logic_vector(63 downto 0);
			freg27	: out std_logic_vector(63 downto 0);
			freg28	: out std_logic_vector(63 downto 0);
			freg29	: out std_logic_vector(63 downto 0);
			freg30	: out std_logic_vector(63 downto 0);
			freg31	: out std_logic_vector(63 downto 0)
		);
	END COMPONENT;

	signal raddr1_reg : std_logic_vector(4 downto 0);
	signal raddr2_reg : std_logic_vector(4 downto 0);
	signal raddr3_reg : std_logic_vector(4 downto 0);
	signal waddr_reg  : std_logic_vector(4 downto 0);
	signal we_reg     : std_logic;
	signal wdata_reg  : std_logic_vector(63 downto 0);
	signal rdata1_reg : std_logic_vector(63 downto 0);
	signal rdata2_reg : std_logic_vector(63 downto 0);
	signal rdata3_reg : std_logic_vector(63 downto 0);

begin
	FREG1 : fregfile_ram
		port map(
			data      => fprwi.wdata,
			rdaddress => fprri.raddr1,
			rdclock   => clk,
			rdclocken => fprri.rclke,
			wraddress => fprwi.waddr,
			wrclock   => clk,
			wrclocken => fprwi.wclke,
			wren      => fprwi.we,
		  dword     => fprwi.dword,
			q         => rdata1_reg,
			freg0	=> open,
			freg1	=> open,
			freg2	=> open,
			freg3	=> open,
			freg4	=> open,
			freg5	=> open,
			freg6	=> open,
			freg7	=> open,
			freg8	=> open,
			freg9	=> open,
			freg10	=> open,
			freg11	=> open,
			freg12	=> open,
			freg13	=> open,
			freg14	=> open,
			freg15	=> open,
			freg16	=> open,
			freg17	=> open,
			freg18	=> open,
			freg19	=> open,
			freg20	=> open,
			freg21	=> open,
			freg22	=> open,
			freg23	=> open,
			freg24	=> open,
			freg25	=> open,
			freg26	=> open,
			freg27	=> open,
			freg28	=> open,
			freg29	=> open,
			freg30	=> open,
			freg31	=> open
		);

	FREG2 : fregfile_ram
		port map(
			data      => fprwi.wdata,
			rdaddress => fprri.raddr2,
			rdclock   => clk,
			rdclocken => fprri.rclke,
			wraddress => fprwi.waddr,
			wrclock   => clk,
			wrclocken => fprwi.wclke,
			wren      => fprwi.we,
		  dword     => fprwi.dword,
			q         => rdata2_reg,
			freg0	=> open,
			freg1	=> open,
			freg2	=> open,
			freg3	=> open,
			freg4	=> open,
			freg5	=> open,
			freg6	=> open,
			freg7	=> open,
			freg8	=> open,
			freg9	=> open,
			freg10	=> open,
			freg11	=> open,
			freg12	=> open,
			freg13	=> open,
			freg14	=> open,
			freg15	=> open,
			freg16	=> open,
			freg17	=> open,
			freg18	=> open,
			freg19	=> open,
			freg20	=> open,
			freg21	=> open,
			freg22	=> open,
			freg23	=> open,
			freg24	=> open,
			freg25	=> open,
			freg26	=> open,
			freg27	=> open,
			freg28	=> open,
			freg29	=> open,
			freg30	=> open,
			freg31	=> open
		);

  FREG3 : fregfile_ram
    port map(
      data      => fprwi.wdata,
      rdaddress => fprri.raddr3,
      rdclock   => clk,
      rdclocken => fprri.rclke,
      wraddress => fprwi.waddr,
      wrclock   => clk,
      wrclocken => fprwi.wclke,
      wren      => fprwi.we,
      dword     => fprwi.dword,
      q         => rdata3_reg,
      freg0   => open,
      freg1   => open,
      freg2   => open,
      freg3   => open,
      freg4   => open,
      freg5   => open,
      freg6   => open,
      freg7   => open,
      freg8   => open,
      freg9   => open,
      freg10  => open,
      freg11  => open,
      freg12  => open,
      freg13  => open,
      freg14  => open,
      freg15  => open,
      freg16  => open,
      freg17  => open,
      freg18  => open,
      freg19  => open,
      freg20  => open,
      freg21  => open,
      freg22  => open,
      freg23  => open,
      freg24  => open,
      freg25  => open,
      freg26  => open,
      freg27  => open,
      freg28  => open,
      freg29  => open,
      freg30  => open,
      freg31  => open
    );

	process(clk, rst_n)
	begin
		if rst_n = '0' then
			raddr1_reg <= (others => '0');
			raddr2_reg <= (others => '0');
			raddr3_reg <= (others => '0');
			waddr_reg  <= (others => '0');
			wdata_reg  <= (others => '0');
			we_reg     <= '0';
		elsif (rising_edge(clk)) then
			if global_stall = '0' then
				if (fprri.rclke = '1') then
					raddr1_reg <= fprri.raddr1;
					raddr2_reg <= fprri.raddr2;
					raddr3_reg <= fprri.raddr3;
				end if;
				if (fprwi.wclke = '1') then
					waddr_reg <= fprwi.waddr;
					wdata_reg <= fprwi.wdata;
					we_reg    <= fprwi.we;
				end if;
			end if;
		end if;
	end process;

	process(raddr1_reg, raddr2_reg, raddr3_reg, waddr_reg, we_reg, wdata_reg, rdata1_reg, rdata2_reg, rdata3_reg)
	begin
		if we_reg = '1' and waddr_reg = raddr1_reg then
			fpro.fdata1 <= wdata_reg;
		else
			fpro.fdata1 <= rdata1_reg;
		end if;

		if we_reg = '1' and waddr_reg = raddr2_reg then
			fpro.fdata2 <= wdata_reg;
		else
			fpro.fdata2 <= rdata2_reg;
		end if;

		if we_reg = '1' and waddr_reg = raddr3_reg then
			fpro.fdata3 <= wdata_reg;
		else
			fpro.fdata3 <= rdata3_reg;
		end if;
	end process;

end rtl;
