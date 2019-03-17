library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libproc.all;
use work.libeu.all;
USE work.LibNode.ALL;

entity eu is                            -- execution unit

	generic(
		id    : integer;
		count : integer;
		nocdim : std_logic_vector(63 downto 0)
	);
	port(
		clk            : in  std_logic;
		rst_n          : in  std_logic;
		ico            : in  icache_out_type;
		ici            : out icache_in_type;
		dco            : in  dcache_out_type;
		dci            : out dcache_in_type;
		nbo            : out P_PORT_BUFFER;
		nbsf           : in  std_logic;
		NodeToBuffer   : in P_PORT_BUFFER;
        NodeToBuffer2 : in P_PORT_BUFFER;
        RecvBufferEmpty : in std_logic;
		n_global_stall : out std_logic;
        LocalRdyIn_1    : in RdyAddress;
        LocalRdyIn_2    : in RdyAddress;
        recvClear      : out std_logic;
        
        BarrierConfigOut : out BarrierConfig; 
        BarrierSetIn   : in std_logic
	);

end entity eu;

architecture rtl of eu is
	signal rfri : regfile_read_in_type;
	signal rfwi : regfile_write_in_type;
	signal rfo  : regfile_out_type;

	signal csrri : csrfile_read_in_type;
	signal csrwi : csrfile_write_in_type;
	signal csro  : csrfile_out_type;

	signal fqo : fetch_out_type;
	signal fyo : fetch_out_type;
	signal dqo : decode_out_type;
	signal dyo : decode_out_type;
	signal eqo : execute_out_type;
	signal eyo : execute_out_type;
	signal mqo : memory_out_type;
	signal myo : memory_out_type;
	signal wqo : writeback_out_type;
	signal wyo : writeback_out_type;

	signal global_stall : std_logic;
begin
	REG1 : regfile port map(
			clk          => clk,
			rst_n        => rst_n,
			rfri         => rfri,
			rfwi         => rfwi,
			rfo          => rfo,
			global_stall => global_stall
		);

	CSR1 : csrfile
		generic map(
			id    => id,
			count => count,
			nocdim => nocdim
		)
		port map(
			clk          => clk,
			rst_n        => rst_n,
			csrri        => csrri,
			csrwi        => csrwi,
			csro         => csro,
			global_stall => global_stall
		);

	IF1 : fetch_stage port map(
			clk          => clk,
			rst_n        => rst_n,
			csro         => csro,
			ico          => ico,
			ici          => ici,
			d.d          => dqo,
			d.e          => eqo,
			d.m          => mqo,
			d.w          => wqo,
			a.d          => dyo,
			a.e          => eyo,
			a.m          => myo,
			a.w          => wyo,
			q            => fqo,
			y            => fyo,
			global_stall => global_stall
		);

	ID1 : decode_stage port map(
			clk          => clk,
			rst_n        => rst_n,
			rfo          => rfo,
			rfri         => rfri,
			csro         => csro,
			csrri        => csrri,
			d.f          => fqo,
			d.e          => eqo,
			d.m          => mqo,
			d.w          => wqo,
			a.f          => fyo,
			a.e          => eyo,
			a.m          => myo,
			a.w          => wyo,
			q            => dqo,
			y            => dyo,
			global_stall => global_stall
		);

	EX1 : execute_stage port map(
			clk          => clk,
			rst_n        => rst_n,
			d.d          => dqo,
			d.m          => mqo,
			d.w          => wqo,
			a.d          => dyo,
			a.m          => myo,
			a.w          => wyo,
			q            => eqo,
			y            => eyo,
			global_stall => global_stall,
			sendFull	=> nbsf,
			NodeToBuffer => NodeToBuffer,
            NodeToBuffer2 => NodeToBuffer2,
            RecvBufferEmpty => RecvBufferEmpty,
			nbo	=> nbo,
			LocalRdyIn_1 => LocalRdyIn_1,
			LocalRdyIn_2 => LocalRdyIn_2,
			recvClear => recvClear,
			BarrierConfigOut => BarrierConfigOut,
            BarrierSetIn  => BarrierSetIn
		);

	MEM1 : memory_stage port map(
			clk              => clk,
			rst_n            => rst_n,
			dco              => dco,
			dci              => dci,
			d.e              => eqo,
			d.w              => wqo,
			a.e              => eyo,
			a.w              => wyo,
			q                => mqo,
			y                => myo,
			global_stall     => global_stall,
			global_stall_out => global_stall
		);

	WB1 : writeback_stage port map(
			clk          => clk,
			rst_n        => rst_n,
			rfwi         => rfwi,
			csrwi        => csrwi,
			d.m          => mqo,
			a.m          => myo,
			q            => wqo,
			y            => wyo,
			global_stall => global_stall
		);

	n_global_stall <= not global_stall;
end architecture;
