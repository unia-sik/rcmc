--Arraybased Buffer with FFS search

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;
USE IEEE.math_real.ALL;

ENTITY NetworkInterfaceReceive IS
	PORT (
		Clk : IN STD_LOGIC;
		Rst : IN STD_LOGIC; 
		NodeToBuffer : IN P_PORT_BUFFER;
		StallNode : OUT STD_LOGIC;
		BufferToProcessor : OUT P_PORT_BUFFER;
		ProcessorRequest : IN REQUEST_PORT 
	);
END NetworkInterfaceReceive;
ARCHITECTURE Behavioral OF NetworkInterfaceReceive IS
 
--Record for the data
TYPE T_DATA_CELL IS
RECORD 
	Data : STD_LOGIC_VECTOR (Data_Length - 1 DOWNTO 0);
	Core_Address : STD_LOGIC_VECTOR((Address_Length_X + Address_Length_Y) - 1 DOWNTO 0);
END RECORD;
 
TYPE T_DATA_ARRAY IS ARRAY (0 TO HeapSize - 1) OF T_DATA_CELL;

SIGNAL Data_Array : T_DATA_ARRAY;
SIGNAL Tail_Pointer : NATURAL;
 
BEGIN
	PROCESS (Clk, Rst) 
VARIABLE match : BOOLEAN; 
VARIABLE V_Data_Array : T_DATA_ARRAY;
VARIABLE V_Tail_Pointer : NATURAL;
VARIABLE matchPosition : NATURAL;

BEGIN
	IF Rst = '0' THEN
		Tail_Pointer <= 0; 
		StallNode <= '0';
		match := false;
	ELSIF rising_edge(Clk) THEN
 
		--Set everything to 0
		BufferToProcessor.Data <= (OTHERS => '0');
		BufferToProcessor.DataAvailable <= '0';
		BufferToProcessor.Address <= (others => '0');
		match := false;

		V_Data_Array := Data_Array;
		V_Tail_Pointer := Tail_Pointer;

		--Handle Incoming
		IF (NodeToBuffer.DataAvailable = '1') THEN
			IF (HeapSize /= V_Tail_Pointer) THEN
				V_Data_Array(V_Tail_Pointer).Core_Address := NodeToBuffer.Address;
				V_Data_Array(V_Tail_Pointer).Data := NodeToBuffer.Data;
 
				--Point to next Cell
				V_Tail_Pointer := V_Tail_Pointer + 1;
			END IF; 
		END IF;
 
		-- Request incoming?
		IF ProcessorRequest.Request = '1' THEN
			-- Specific or unspecific request?
			IF ProcessorRequest.SpecificRequest = '1' THEN
				-- Specific request. Find slot.
				FOR i IN V_Data_Array'LOW TO V_Data_Array'HIGH LOOP
					IF V_Data_Array(i).Core_Address = ProcessorRequest.Address AND i < V_Tail_Pointer THEN
						matchPosition := i;
						match := true;
						EXIT;
					END IF;
				END LOOP;

			ELSE
				-- No specific request. Use first slot but only if we have at least one item.
				IF V_Tail_Pointer > 0 THEN
					matchPosition := 0;
					match := true;
				END IF;
			END IF;

			IF match THEN
				BufferToProcessor.DataAvailable <= '1';
				BufferToProcessor.Address <= V_Data_Array(matchPosition).Core_Address;
				BufferToProcessor.Data <= V_Data_Array(matchPosition).Data;
			ELSE
				BufferToProcessor.DataAvailable <= '0';
				BufferToProcessor.Data <= (others => '0');
				BufferToProcessor.Address <= (others => '0');
			END IF;

			
		END IF;
		

		IF V_Tail_Pointer >= HeapSize-1 THEN
			StallNode <= '1';
		ELSE
			StallNode <= '0';
		END IF;

		-- Delete if neccessary.
			IF ProcessorRequest.Request = '1' AND ProcessorRequest.ProbeRequest = '0' AND match THEN
				--Move everything one cell down in the array
				FOR j IN V_Data_Array'LOW TO V_Data_Array'HIGH - 1 LOOP 
					IF (j >= matchPosition) THEN
						V_Data_Array(j) := V_Data_Array(j + 1);
					END IF; 
				END LOOP; 

				--Move Tailpointer down
				V_Tail_Pointer := V_Tail_Pointer - 1;
			END IF;

		Data_Array <= V_Data_Array;
		Tail_Pointer <= V_Tail_Pointer;
	END IF; 
END PROCESS;
END Behavioral;