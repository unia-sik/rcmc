LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;

ENTITY Node IS
    PORT (	Clk 				: IN std_logic;
			Rst 				: IN std_logic;				
			NorthOut			: OUT P_PORT_VERTICAL;
			SouthIn				: IN  P_PORT_VERTICAL;
			EastOut	   			: OUT P_PORT_HORIZONTAL;
			WestIn				: IN  P_PORT_HORIZONTAL;
			LocalOut			: OUT P_PORT_BUFFER;
			LocalIn				: IN  P_PORT_BUFFER;		
			BufferOverflow 		: OUT std_logic;	
			RecvBufferFull 		: IN std_logic;			
			CoreAddress			: IN  Address;
			ProcessorToBuffer   : IN P_PORT_BUFFER;
            SendBufferFull      : OUT std_logic				
		);
END Node;




ARCHITECTURE Behavioral OF Node IS

TYPE PORT_BEHAVIOUR IS (EMPTY, INJECT_FROM_LOCAL, FORWARD_FROM_WEST, DEFLECT_FROM_WEST, FROM_CORNER, FORWARD_FROM_SOUTH, DEFLECT_FROM_SOUTH, EJECT_FROM_SOUTH, EJECT_FROM_WEST, EJECT_FROM_CORNER, TURN_FROM_WEST);

--FIFO for buffering address and data
TYPE T_FIFO IS ARRAY (0 TO Corner_Buffer_Size-1) OF FIFOEntry;
SIGNAL FIFO 				 : T_FIFO;
SIGNAL FIFO_TAIL_POINTER : natural;
SIGNAL FIFO_HEAD_POINTER : natural;
SIGNAL FIFO_EMPTY			 : boolean;

--FIFO for send buffer
TYPE T_SB_FIFO IS ARRAY (0 TO FifoSendBufferSize-1) OF STD_LOGIC_VECTOR((Address_Length_X + Address_Length_Y) + Data_Length -1 DOWNTO 0);
SIGNAL SB_FIFO                             : T_SB_FIFO;
SIGNAL SB_FIFO_TAIL_POINTER        : natural;
SIGNAL SB_FIFO_HEAD_POINTER        : natural;
SIGNAL SB_FIFO_EMPTY                       : boolean;

BEGIN

	PROCESS (Clk, Rst)

	VARIABLE north : PORT_BEHAVIOUR;
	VARIABLE east : PORT_BEHAVIOUR;
	VARIABLE corner : PORT_BEHAVIOUR;
	VARIABLE local : PORT_BEHAVIOUR;

	VARIABLE stall_west 	: natural;
	VARIABLE stall_east 	: natural;
	VARIABLE stall_x 		: natural;
	VARIABLE stall_north 	: natural;
	VARIABLE stall_south 	: natural;
	VARIABLE stall_y 		: natural;
	VARIABLE local_in  		: P_PORT_BUFFER;
	VARIABLE local_used		: boolean;

	VARIABLE FIFO_WR_DATA1_EN		: BOOLEAN;
	VARIABLE FIFO_TAIL_TMP			: NATURAL;
	VARIABLE FIFO_FILL_LEVEL 		: NATURAL;
	VARIABLE FIFO_ITEM_POPPED 		: BOOLEAN;
	VARIABLE FIFO_WR_DATA1 			: FIFOEntry;

	VARIABLE SB_FIFO_ITEM_POPPED               : BOOLEAN;
    VARIABLE SB_FIFO_WR_DATA1                  : STD_LOGIC_VECTOR((Address_Length_X + Address_Length_Y) + Data_Length -1 DOWNTO 0);
    VARIABLE SB_FIFO_WR_DATA1_EN               : BOOLEAN;      
    VARIABLE SB_FIFO_TAIL_TMP                  : NATURAL;
    VARIABLE SB_FIFO_FILL_LEVEL                : INTEGER;


	
	BEGIN	

		IF RST = '0' THEN
			north  := EMPTY;
			east   := EMPTY;
			corner := EMPTY;
			local  := EMPTY;

			stall_west := 0;
			stall_east := 0;
			stall_x := 0;
			stall_north := 0;
			stall_south := 0;
			stall_y := 0;

			BufferOverflow <= '0';
			SendBufferFull <= '0';

			-- CB FIFO
			FIFO_TAIL_POINTER			<= Corner_Buffer_Size-1;
			FIFO_HEAD_POINTER			<= 0;
			FIFO_ITEM_POPPED			:= FALSE;
			FIFO_EMPTY					<= TRUE;
			FIFO_FILL_LEVEL 			:= 0;
			FIFO_WR_DATA1_EN 			:= FALSE;

			-- SB FIFO
			SB_FIFO_TAIL_POINTER                       <= 0;
            SB_FIFO_HEAD_POINTER                       <= 0;
            SB_FIFO_ITEM_POPPED                        := FALSE;
            SB_FIFO_WR_DATA1                           := (OTHERS=>'0');
            SB_FIFO_EMPTY                              <= TRUE;
            SB_FIFO_FILL_LEVEL                         := 0;
            SB_FIFO_WR_DATA1_EN                        := FALSE;


			NorthOut.DataAvailable 	<= '0';
			NorthOut.Marked			<= '0';
			EastOut.DataAvailable 	<= '0';
			EastOut.Marked			<= '0';
			LocalOut.DataAvailable 	<= '0';

		ELSIF rising_edge(CLK) THEN

			-- Rewrite of pnconfig in VHDL.
			-- Flags are ubcc00 for now.
			-- Other flags not supported yet.

			-- FIFO should initially do nothing			
			FIFO_ITEM_POPPED := FALSE;
			FIFO_WR_DATA1_EN := FALSE;

			SB_FIFO_ITEM_POPPED := FALSE;
            SB_FIFO_WR_DATA1_EN := FALSE;

			--Write to FIFO
            IF ProcessorToBuffer.DataAvailable = '1' THEN
              SB_FIFO_WR_DATA1 := ProcessorToBuffer.Address & ProcessorToBuffer.Data;
              SB_FIFO_WR_DATA1_EN := TRUE;
            END IF;

            IF SB_FIFO_TAIL_POINTER /= SB_FIFO_HEAD_POINTER THEN       
            	local_in.Address := SB_FIFO(SB_FIFO_HEAD_POINTER)((Address_Length_X + Address_Length_Y) + Data_Length-1 DOWNTO Data_Length);
            	local_in.Data := SB_FIFO(SB_FIFO_HEAD_POINTER)(Data_Length-1 DOWNTO 0);   
            	local_in.DataAvailable := '1';
            ELSE
                local_in.DataAvailable := '0';
                local_in.Address := (others => '0');
                local_in.Data := (others => '0');
            END IF;

			local_used := false;

			-- Decrease all stall counters.
			IF stall_south /= 0 THEN
				stall_south := stall_south - 1;
			END IF;

			IF SouthIn.DataAvailable = '1' AND SouthIn.Address.AddressSender.Y = CoreAddress.Y AND SouthIn.Marked = '1' THEN
				stall_y := Dimension;
			ELSIF stall_y /= 0 THEN
				stall_y := stall_y - 1;
			END IF;

			IF stall_west /= 0 THEN
				stall_west := stall_west - 1;
			END IF;

			IF WestIn.DataAvailable = '1' AND WestIn.Address.AddressSender.X = CoreAddress.X AND WestIn.Marked = '1' THEN
				stall_x := Dimension;
			ELSIF stall_x /= 0 THEN
				stall_x := stall_x - 1;
			END IF;

			IF stall_north /= 0 THEN
				stall_north := stall_north - 1;
			ELSIF SouthIn.Marked = '1' THEN
				stall_north := Dimension;
			END IF;

			IF stall_east /= 0 THEN
				stall_east := stall_east - 1;
			ELSIF WestIn.Marked = '1' THEN
				stall_east := Dimension;
			END IF;

			-- south to north decisions
			north  := EMPTY;
			local  := EMPTY;

			IF SouthIn.DataAvailable = '1' THEN
				IF SouthIn.Address.AddressReceiver.Y /= CoreAddress.Y THEN
					north := FORWARD_FROM_SOUTH;
				ELSIF stall_south /= 0 THEN
					north := DEFLECT_FROM_SOUTH;
				ELSIF RecvBufferFull = '1' THEN
					stall_south := Dimension;
					north := DEFLECT_FROM_SOUTH;
				ELSE
					local := EJECT_FROM_SOUTH;
				END IF;
			END IF;

			-- corner to north if empty
			-- all conditions hardcoded, including special case for unbuffered
			IF FIFO_FILL_LEVEL /= 0 AND FIFO(FIFO_HEAD_POINTER).Address.AddressReceiver /= CoreAddress AND stall_y = 0 AND stall_north = 0 AND north = EMPTY THEN
				north := FROM_CORNER;
			END IF;

			east   := EMPTY;
			corner := EMPTY;

			-- west to east
			IF WestIn.DataAvailable = '1' THEN
				IF WestIn.Address.AddressReceiver.X /= CoreAddress.X THEN
					east := FORWARD_FROM_WEST;
				ELSIF stall_west /= 0 THEN
					east := DEFLECT_FROM_WEST;
				ELSIF WestIn.Address.AddressReceiver.Y = CoreAddress.Y THEN
					IF RecvBufferFull = '1' OR local /= EMPTY THEN
						stall_west := Dimension;
						east := DEFLECT_FROM_WEST;
					ELSE
						local := EJECT_FROM_WEST;
					END IF;
				ELSIF FIFO_FILL_LEVEL = Corner_Buffer_Size THEN
					stall_west := Dimension;
					east := DEFLECT_FROM_WEST;
				ELSE
					corner := TURN_FROM_WEST;
				END IF;
			END IF;

			-- [ direct ejection from CB not here because only needed for buf or 2buf ]

			IF local_in.DataAvailable = '1' THEN
				IF local_in.Address(Address_Length_X+Address_Length_Y-1 DOWNTO Address_Length_Y) = CoreAddress.X THEN
					IF FIFO_FILL_LEVEL /= Corner_Buffer_Size AND corner = EMPTY THEN
						corner := INJECT_FROM_LOCAL;
					END IF;
				ELSE
					IF stall_x = 0 AND stall_east = 0 AND east = EMPTY THEN
						east := INJECT_FROM_LOCAL;
					END IF;
				END IF;
			END IF;


			
			CASE north IS
				WHEN FORWARD_FROM_SOUTH =>
					NorthOut.DataAvailable <= '1';
					NorthOut.Marked <= SouthIn.Marked;
					NorthOut.Data <= SouthIn.Data;
					NorthOut.Address <= SouthIn.Address;
				WHEN DEFLECT_FROM_SOUTH =>
					NorthOut.DataAvailable <= '1';
					NorthOut.Marked <= '1';
					NorthOut.Data <= SouthIn.Data;
					NorthOut.Address <= SouthIn.Address;
				WHEN INJECT_FROM_LOCAL =>
					NorthOut.DataAvailable <= '1';
					NorthOut.Marked <= '0';
					NorthOut.Data <= local_in.Data;
					NorthOut.Address.AddressReceiver.X <= local_in.Address(Address_Length_X+Address_Length_Y-1 DOWNTO Address_Length_Y);
					NorthOut.Address.AddressReceiver.Y <= local_in.Address(Address_Length_Y-1 DOWNTO 0);
					NorthOut.Address.AddressSender.X <= CoreAddress.X;
					NorthOut.Address.AddressSender.Y <= CoreAddress.Y;
					local_used := TRUE;
				WHEN FROM_CORNER =>
					NorthOut.DataAvailable <= '1';
					NorthOut.Marked <= '0';
					NorthOut.Data <= FIFO(FIFO_HEAD_POINTER).Data;
					NorthOut.Address <= FIFO(FIFO_HEAD_POINTER).Address;
					FIFO_ITEM_POPPED := TRUE;
				WHEN OTHERS =>
					NorthOut.DataAvailable <= '0';
					NorthOut.Marked <= '0';
			END CASE;

			CASE east IS
				WHEN FORWARD_FROM_WEST =>
					EastOut.DataAvailable <= '1';
					EastOut.Marked <= WestIn.Marked;
					EastOut.Data <= WestIn.Data;
					EastOut.Address <= WestIn.Address;
				WHEN DEFLECT_FROM_WEST =>
					EastOut.DataAvailable <= '1';
					EastOut.Marked <= '1';
					EastOut.Data <= WestIn.Data;
					EastOut.Address <= WestIn.Address;
				WHEN INJECT_FROM_LOCAL =>
					EastOut.DataAvailable <= '1';
					EastOut.Marked <= '0';
					EastOut.Data <= local_in.Data;
					EastOut.Address.AddressReceiver.X <= local_in.Address(Address_Length_X+Address_Length_Y-1 DOWNTO Address_Length_Y);
					EastOut.Address.AddressReceiver.Y <= local_in.Address(Address_Length_Y-1 DOWNTO 0);
					EastOut.Address.AddressSender.X <= CoreAddress.X;
					EastOut.Address.AddressSender.Y <= CoreAddress.Y;	
					local_used := TRUE;
				WHEN OTHERS =>
					EastOut.DataAvailable <= '0';
					EastOut.Marked <= '0';
			END CASE;

			CASE local IS
				WHEN EJECT_FROM_SOUTH =>
					LocalOut.DataAvailable <= '1';
					LocalOut.Data <= SouthIn.Data;
					LocalOut.Address <= SouthIn.Address.AddressSender.X & SouthIn.Address.AddressSender.Y;
				WHEN EJECT_FROM_WEST =>
					LocalOut.DataAvailable <= '1';
					LocalOut.Data <= WestIn.Data;
					LocalOut.Address <= WestIn.Address.AddressSender.X & WestIn.Address.AddressSender.Y;
				WHEN EJECT_FROM_CORNER =>
					LocalOut.DataAvailable <= '1';
					LocalOut.Data <= FIFO(FIFO_HEAD_POINTER).Data;
					LocalOut.Address <= FIFO(FIFO_HEAD_POINTER).Address.AddressSender.X & FIFO(FIFO_HEAD_POINTER).Address.AddressSender.Y;
					FIFO_ITEM_POPPED := TRUE;
				WHEN OTHERS =>
					LocalOut.DataAvailable <= '0';
			END CASE;



			CASE corner IS
				WHEN TURN_FROM_WEST => 
					FIFO_WR_DATA1.Data := WestIn.Data;						
					FIFO_WR_DATA1.Address := WestIn.Address;		
					FIFO_WR_DATA1_EN := TRUE;
				WHEN INJECT_FROM_LOCAL => 
					FIFO_WR_DATA1.Data := local_in.Data;						
					FIFO_WR_DATA1.Address.AddressReceiver.X := local_in.Address(Address_Length_X+Address_Length_Y-1 DOWNTO Address_Length_Y);
					FIFO_WR_DATA1.Address.AddressReceiver.Y := local_in.Address(Address_Length_Y-1 DOWNTO 0);
					FIFO_WR_DATA1.Address.AddressSender.X := CoreAddress.X;
					FIFO_WR_DATA1.Address.AddressSender.Y := CoreAddress.Y;		
					FIFO_WR_DATA1_EN := TRUE;
					local_used := TRUE;
				WHEN OTHERS =>
					NULL;
			END CASE;

--FIFO BEGIN
-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
			-- If item was removed from FIFO, move the Headpointer
			IF FIFO_ITEM_POPPED THEN
				FIFO_HEAD_POINTER <= (FIFO_HEAD_POINTER + 1) MOD Corner_Buffer_Size;				
				FIFO_FILL_LEVEL := FIFO_FILL_LEVEL-1;				
			END IF;

			FIFO_TAIL_TMP := FIFO_TAIL_POINTER;

			
			--Insert item(s) into FIFO				
			IF FIFO_WR_DATA1_EN THEN
				FIFO_TAIL_TMP := (FIFO_TAIL_TMP+1) MOD Corner_Buffer_size;
				FIFO(FIFO_TAIL_TMP) <= FIFO_WR_DATA1;
				FIFO_FILL_LEVEL := FIFO_FILL_LEVEL+1;				
			END IF;
		
			
			--No change to the pointer if no data was written
			FIFO_TAIL_POINTER <= FIFO_TAIL_TMP;
			
			--Overflow detection			
			IF FIFO_FILL_LEVEL >= Corner_Buffer_Size THEN
				BufferOverflow <= '1';
			ELSE
				BufferOverflow <= '0';
			END IF;
			
			
			--FIFO empty?			
			IF FIFO_FILL_LEVEL = 0 THEN
				FIFO_EMPTY <= TRUE;					
			ELSE
				FIFO_EMPTY <= FALSE;					
			END IF;			
			
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			
--FIFO END

			IF SB_FIFO_TAIL_POINTER /= SB_FIFO_HEAD_POINTER AND local_used THEN       
            	SB_FIFO_ITEM_POPPED := TRUE;
            --ELSIF SB_FIFO_TAIL_POINTER = SB_FIFO_HEAD_POINTER AND ProcessorToBuffer.DataAvailable = '1' AND local_used THEN
            --    SB_FIFO_WR_DATA1_EN := FALSE;
            END IF;

 --If item was removed from FIFO, move the Headpointer
               IF SB_FIFO_ITEM_POPPED THEN
                       SB_FIFO_HEAD_POINTER <= (SB_FIFO_HEAD_POINTER + 1) MOD FifoSendBufferSize;                            
                       SB_FIFO_FILL_LEVEL := SB_FIFO_FILL_LEVEL-1;                           
               END IF;
                                               
                       
               SB_FIFO_TAIL_TMP := SB_FIFO_TAIL_POINTER;
                       
                       
               --Insert item(s) into FIFO                              
               IF SB_FIFO_WR_DATA1_EN AND SB_FIFO_FILL_LEVEL < FifoSendBufferSize THEN
                       
                       SB_FIFO(SB_FIFO_TAIL_TMP) <= SB_FIFO_WR_DATA1;
                       SB_FIFO_TAIL_TMP := (SB_FIFO_TAIL_TMP+1) MOD FifoSendBufferSize;
                       SB_FIFO_FILL_LEVEL := SB_FIFO_FILL_LEVEL+1;                           
               END IF;
               
                       
                       
               --No change to the pointer if no data was written
               SB_FIFO_TAIL_POINTER <= SB_FIFO_TAIL_TMP;
                       
               --SendBufferFull detection                      
               IF SB_FIFO_FILL_LEVEL >= FifoSendBufferSize-1 THEN
               	SendBufferFull <= '1';
               ELSE
               	SendBufferFull <= '0';
               END IF;
		END IF;

	END PROCESS;
	
END Behavioral;
