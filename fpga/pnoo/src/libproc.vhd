library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
USE work.LibNode.ALL;
library work;

package libproc is

	component cpu_top
		generic (
			id    : integer;
			count : integer;
			nocdim : std_logic_vector(63 downto 0)
		);
	port (
		clk	: in  std_logic;
		rst_n	: in  std_logic;
		nbo : out P_PORT_BUFFER;
		nbsf	: in std_logic;
		NodeToBuffer	: in       P_PORT_BUFFER;
        NodeToBuffer2 : in P_PORT_BUFFER;        
        RecvBufferEmpty : in std_logic;
        
        BarrierConfigOut : out BarrierConfig; 
        BarrierSetIn   : in std_logic;
        
        LocalRdyIn_1 : in RdyAddress;
        LocalRdyIn_2 : in RdyAddress;
        recvClear      : out std_logic
	);
	end component cpu_top;
	
	type icache_in_type is record
		addr	: std_logic_vector(63 downto 0);
	end record;
	
	type icache_out_type is record
		data		: std_logic_vector(31 downto 0);
	end record;

	type dcache_in_type is record
		addr		: std_logic_vector(63 downto 0);
		wdata		: std_logic_vector(63 downto 0);
		we			: std_logic;
		byteen	: std_logic_vector( 7 downto 0);
	end record;
	
	type dcache_out_type is record
		rdata		: std_logic_vector(63 downto 0);
	end record;
	
	component eu
		generic (
			id    : integer;
			count : integer;
			nocdim : std_logic_vector(63 downto 0)
		);
		port (
			clk	: in  std_logic;
			rst_n	: in  std_logic;
			ico	: in  icache_out_type;
			ici	: out icache_in_type;
			dco	: in  dcache_out_type;
			dci	: out dcache_in_type;
			nbo 	: out P_PORT_BUFFER;
			nbsf 	: in std_logic;
			NodeToBuffer	: in P_PORT_BUFFER;
            NodeToBuffer2 : in P_PORT_BUFFER;
            RecvBufferEmpty : in std_logic;
			n_global_stall : out std_logic;
            LocalRdyIn_1 : in RdyAddress;
            LocalRdyIn_2 : in RdyAddress;
            recvClear      : out std_logic;
            BarrierConfigOut : out BarrierConfig; 
            BarrierSetIn   : in std_logic
		);
	end component eu;
	
	component dmem
		port (
            rst : IN std_logic;    
			clk	: in  std_logic;
			ici	: in  icache_in_type;
			ico	: out icache_out_type;
			dci	: in  dcache_in_type;
			dco	: out dcache_out_type;
			clken : in std_logic
		);
	end component dmem;
	
end package libproc;
