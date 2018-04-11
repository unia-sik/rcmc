library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libproc.all;
use work.libnode.all;

entity nocunit is
	generic(
		id     : integer;
		count  : integer;
		nocdim : std_logic_vector(63 downto 0)
	);
	port(
		clk         : in  std_logic;    --
		rst_n       : in  std_logic;    --
		LED        : out std_logic;    --
		NorthIn     : in  P_PORT_VERTICAL_BACK; --
		NorthOut    : out P_PORT_VERTICAL; --
		SouthIn     : in  P_PORT_VERTICAL; --
		SouthOut    : out P_PORT_VERTICAL_BACK; --
		EastIn      : IN  P_PORT_HORIZONTAL_BACK; --
		EastOut     : out P_PORT_HORIZONTAL; --
		WestIn      : in  P_PORT_HORIZONTAL; --
		WestOut     : out P_PORT_HORIZONTAL_BACK; --
		CoreAddress : IN  Address
	);

end entity nocunit;

architecture rtl of nocunit is
	signal BufferToNode : P_PORT_BUFFER;
	signal NodeToBuffer : P_PORT_BUFFER;

	signal BufferToProcessor : P_PORT_BUFFER;
	signal ProcessorToBuffer : P_PORT_BUFFER;
	signal ProcessorRequest  : REQUEST_PORT;
	signal SendBufferFull    : std_logic;
	signal RecvBufferFull    : std_logic;

begin
	LED <= SendBufferFull;
	cpu_inst : cpu_top
		generic map(
			id     => id,
			count  => count,
			nocdim => nocdim
		)
		port map(
			clk   => clk,
			rst_n => rst_n,
			nbi   => BufferToProcessor,
			nbo   => ProcessorToBuffer,
			nbr   => ProcessorRequest,
			nbsf  => SendBufferFull
		);

	nir_inst : NetworkInterfaceReceive
		port map(
			Clk               => clk,
			Rst               => rst_n,
			NodeToBuffer      => NodeToBuffer,
			BufferToProcessor => BufferToProcessor,
			ProcessorRequest  => ProcessorRequest,
			StallNode         => RecvBufferFull
		);

	node_inst : Node
		port map(
			Clk               => clk,
			Rst               => rst_n,
			NorthIn           => NorthIn,
			NorthOut          => NorthOut,
			SouthIn           => SouthIn,
			SouthOut          => SouthOut,
			EastIn            => EastIn,
			EastOut           => EastOut,
			WestIn            => WestIn,
			WestOut           => WestOut,
			LocalOut          => NodeToBuffer,
			LocalIn           => BufferToNode,
			RecvBufferFull    => RecvBufferFull,
			CoreAddress       => CoreAddress,
			ProcessorToBuffer => ProcessorToBuffer,
			SendBufferFull    => SendBufferFull
		);

end architecture;