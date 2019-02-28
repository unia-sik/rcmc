library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libproc.all;
USE work.LibNode.ALL;

entity cpu_top is
	generic(
		id    : integer;
		count : integer;
		nocdim : std_logic_vector(63 downto 0)
	);
	port(
		clk   : in  std_logic;
		rst_n : in  std_logic;
		nbo   : out P_PORT_BUFFER;
		nbsf	: in  std_logic;
		NodeToBuffer	: in P_PORT_BUFFER;
        NodeToBuffer2 : in P_PORT_BUFFER;
        RecvBufferEmpty : in std_logic;
        
        BarrierConfigOut : out BarrierConfig; 
        BarrierSetIn   : in std_logic;
        
        LocalRdyIn_1: in RdyAddress;
        LocalRdyIn_2: in RdyAddress;
        recvClear      : out std_logic
	);

end entity cpu_top;

architecture rtl of cpu_top is
	signal ici : icache_in_type;
	signal ico : icache_out_type;

	signal dci : dcache_in_type;
	signal dco : dcache_out_type;

	signal n_global_stall : std_logic;

	signal clk_inv : std_logic;

begin
	EU1 : eu
		generic map(
			id    => id,
			count => count,
			nocdim => nocdim
		)
		port map(
			clk            => clk,
			rst_n          => rst_n,
			ico            => ico,
			ici            => ici,
			dco            => dco,
			dci            => dci,
			n_global_stall => n_global_stall,
			nbo            => nbo,
			nbsf           => nbsf,
			NodeToBuffer     => NodeToBuffer,
            NodeToBuffer2    => NodeToBuffer2,
            RecvBufferEmpty  => RecvBufferEmpty,            
            BarrierConfigOut => BarrierConfigOut,
            BarrierSetIn     => BarrierSetIn,
            
            LocalRdyIn_1 => LocalRdyIn_1,
            LocalRdyIn_2 => LocalRdyIn_2,
            recvClear => recvClear
		);

	MEM1 : dmem port map(
            rst   => rst_n,
			clk   => clk_inv,
			ici   => ici,
			ico   => ico,
			dci   => dci,
			dco   => dco,
			clken => n_global_stall
		);

	comb : process(clk)
	begin
		clk_inv <= not clk;
	end process;

end architecture;
