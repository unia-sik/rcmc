LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE work.constants.ALL;


PACKAGE LibNode IS   

	type Address is
	record			
		X: std_logic_vector(Address_Length_X-1 DOWNTO 0);
		Y: std_logic_vector(Address_Length_Y-1 DOWNTO 0);
	end record;
	
	type AddressPort is
	record			
		AddressReceiver:	Address;
		AddressSender:		Address;	
	end record;

	type P_PORT_VERTICAL is
	record			
		Data: 				std_logic_vector(Data_Length-1 DOWNTO 0);
		--Address: 			std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
		Address:				AddressPort;	
		DataAvailable: 	std_logic;
		Marked:				std_logic;		
	end record;
	
	type P_PORT_HORIZONTAL is
	record			
		Data: 				std_logic_vector(Data_Length-1 DOWNTO 0);
		--Address: 			std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
		Address:				AddressPort;	
		DataAvailable: 	std_logic;	
		Marked:				std_logic;		
	end record;
	
	TYPE REQUEST_PORT is
	RECORD
		Address:			std_logic_vector((Address_Length_X + Address_Length_Y)-1  DOWNTO 0);
		Request:			std_logic;
		SpecificRequest:	std_logic;              
		ProbeRequest:		std_logic; --als peek, daten rückliefern aber nicht entfernen
	END RECORD;

	type P_PORT_BUFFER is
	record			
		Data: 				std_logic_vector(Data_Length-1 DOWNTO 0);
		Address: 			std_logic_vector((Address_Length_X + Address_Length_Y)-1  DOWNTO 0);
		DataAvailable: 	std_logic;					
	end record;
		
	
	type FIFOEntry is
	record			
		Data: 				std_logic_vector(Data_Length-1 DOWNTO 0);
		Address:				AddressPort;
	end record;
	
	TYPE BusyCounterReceive_T IS ARRAY (0 TO Dimension - 1) OF natural;
	
		component NODE
    PORT (		Clk 					: IN std_logic;
				Rst 					: IN std_logic;
				NorthOut				: OUT P_PORT_VERTICAL;
				SouthIn					: IN  P_PORT_VERTICAL;
				EastOut	   				: OUT P_PORT_HORIZONTAL;
				WestIn					: IN  P_PORT_HORIZONTAL;
				LocalOut				: OUT P_PORT_BUFFER;
				LocalIn					: IN  P_PORT_BUFFER;
				BufferOverflow 			: OUT std_logic;
				RecvBufferFull			: IN std_logic;
				CoreAddress				: IN  Address;
				ProcessorToBuffer		: IN P_PORT_BUFFER;
				SendBufferFull			: OUT std_logic
				);
END component;

	COMPONENT NetworkInterfaceReceive
	PORT (
		Clk : IN STD_LOGIC;
		Rst : IN STD_LOGIC; 
		NodeToBuffer : IN P_PORT_BUFFER;
		StallNode : OUT STD_LOGIC;
		BufferToProcessor : OUT P_PORT_BUFFER;
		ProcessorRequest : IN REQUEST_PORT 
	);
	END COMPONENT;
	
END LibNode;