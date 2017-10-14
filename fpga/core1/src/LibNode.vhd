library ieee;
use ieee.std_logic_1164.all;
use work.constants.all;

package LibNode is
	type Address is record
		X : std_logic_vector(Address_Length_X - 1 downto 0);
		Y : std_logic_vector(Address_Length_Y - 1 downto 0);
	end record;

	type AddressPort is record
		AddressReceiver : Address;
		AddressSender   : Address;
	end record;

	type P_PORT_VERTICAL is record
		Data          : std_logic_vector(Data_Length - 1 downto 0);
		--Address:          std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
		Address       : AddressPort;
		DataAvailable : std_logic;
		Marked        : std_logic;
		--Request       : std_logic;
	end record;

	type P_PORT_VERTICAL_BACK is record
		--Address:          std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
		--Address : AddressPort;
		Request : std_logic;
	end record;

	type P_PORT_HORIZONTAL is record
		Data          : std_logic_vector(Data_Length - 1 downto 0);
		--Address:          std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
		Address       : AddressPort;
		DataAvailable : std_logic;
		Marked        : std_logic;
		--Request       : std_logic;
	end record;

	type P_PORT_HORIZONTAL_BACK is record
		--Address:          std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
		--Address : AddressPort;
		Request : std_logic;
	end record;

	type REQUEST_PORT is record
		Address         : std_logic_vector((Address_Length_X + Address_Length_Y) - 1 downto 0);
		Request         : std_logic;
		SpecificRequest : std_logic;
		ProbeRequest    : std_logic;    --als peek, daten rückliefern aber nicht entfernen
	end record;

	type P_PORT_BUFFER is record
		Data          : std_logic_vector(Data_Length - 1 downto 0);
		Address       : std_logic_vector((Address_Length_X + Address_Length_Y) - 1 downto 0);
		DataAvailable : std_logic;
	end record;

	type FIFOEntry is record
		Data    : std_logic_vector(Data_Length - 1 downto 0);
		Address : AddressPort;
	end record;

	type BusyCounterReceive_T is array (0 to Dimension - 1) of natural;

	component NODE
		port(Clk               : in  std_logic;
			 Rst               : in  std_logic;
		   NorthIn           : in  P_PORT_VERTICAL_BACK;
		   NorthOut          : out P_PORT_VERTICAL;
		   SouthIn           : in  P_PORT_VERTICAL;
		   SouthOut          : out P_PORT_VERTICAL_BACK;
		   EastIn            : IN  P_PORT_HORIZONTAL_BACK;
		   EastOut           : out P_PORT_HORIZONTAL;
		   WestIn            : in  P_PORT_HORIZONTAL;
		   WestOut           : out P_PORT_HORIZONTAL_BACK;
			 LocalOut          : out P_PORT_BUFFER;
			 LocalIn           : in  P_PORT_BUFFER;
			 BufferOverflow    : out std_logic;
			 RecvBufferFull    : in  std_logic;
			 CoreAddress       : in  Address;
			 ProcessorToBuffer : in  P_PORT_BUFFER;
			 SendBufferFull    : out std_logic
		);
	end component;

	component NetworkInterfaceReceive
		port(
			Clk               : in  std_logic;
			Rst               : in  std_logic;
			NodeToBuffer      : in  P_PORT_BUFFER;
			StallNode         : out std_logic;
			BufferToProcessor : out P_PORT_BUFFER;
			ProcessorRequest  : in  REQUEST_PORT
		);
	end component;

end LibNode;
