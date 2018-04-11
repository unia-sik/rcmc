library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
USE work.LibNode.ALL;

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
		nbi : in P_PORT_BUFFER;
		nbo : out P_PORT_BUFFER;
		nbr : out REQUEST_PORT;
		nbsf : in std_logic
	);
	end component cpu_top;
    
    type MCodeInstr is array (0 to 3) of std_logic_vector (31 downto 0);
	
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
	
	type mcodecache_in_type is record
		addr	: std_logic_vector(63 downto 0);
	end record;
	
	type mcodecache_out_type is record
		data : MCodeInstr;
	end record;
	
	component eu
		generic (
			id    : integer;
			count : integer;
			nocdim : std_logic_vector(63 downto 0)
		);
		port (
			clk		: in  std_logic;
			rst_n	: in  std_logic;
			ico		: in  icache_out_type;
			ici		: out icache_in_type;
			dco		: in  dcache_out_type;
			dci		: out dcache_in_type;
			mcco	: in  mcodecache_out_type;
			mcci	: out mcodecache_in_type;
			nbi 	: in P_PORT_BUFFER;
			nbo 	: out P_PORT_BUFFER;
			nbr		: out REQUEST_PORT;
			nbsf 	: in std_logic;
			n_global_stall : out std_logic
		);
	end component eu;
	
	component dmem
		port (
			clk	: in  std_logic;
			ici	: in  icache_in_type;
			ico	: out icache_out_type;
			dci	: in  dcache_in_type;
			dco	: out dcache_out_type;
			clken : in std_logic
		);
	end component dmem;
	
	
	component mcodecache
	    port(
			clk          : in  std_logic;
			mcci         : in  mcodecache_in_type;
			mcco         : out mcodecache_out_type;
			clken        : in  std_logic
	    );
	end component mcodecache;
	
end package libproc;