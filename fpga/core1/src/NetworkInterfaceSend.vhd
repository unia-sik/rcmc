--Arraybased Buffer with FFS search

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;
USE IEEE.math_real.ALL;

ENTITY NetworkInterfaceSend IS
       PORT (
               Clk                             : IN STD_LOGIC;
               Rst                             : IN STD_LOGIC;
               ProcessorToBuffer       : IN P_PORT_BUFFER;
               LocalStallSignal        : IN std_logic;
               SendBufferFull          : OUT std_logic;
               BufferToNode            : OUT P_PORT_BUFFER
       );
END NetworkInterfaceSend;

ARCHITECTURE Behavioral OF NetworkInterfaceSend IS

       --FIFO
       TYPE T_FIFO IS ARRAY (0 TO FifoSendBufferSize-1) OF STD_LOGIC_VECTOR((Address_Length_X + Address_Length_Y) + Data_Length -1 DOWNTO 0);
       SIGNAL FIFO                             : T_FIFO;
       SIGNAL FIFO_TAIL_POINTER        : natural;
       SIGNAL FIFO_HEAD_POINTER        : natural;
       SIGNAL FIFO_EMPTY                       : boolean;

BEGIN

PROCESS (Clk, Rst)

       VARIABLE FIFO_ITEM_POPPED               : BOOLEAN;
       VARIABLE FIFO_WR_DATA1                  : STD_LOGIC_VECTOR((Address_Length_X + Address_Length_Y) + Data_Length -1 DOWNTO 0);
       VARIABLE FIFO_WR_DATA1_EN               : BOOLEAN;      
       VARIABLE FIFO_TAIL_TMP                  : NATURAL;
       VARIABLE FIFO_FILL_LEVEL                : INTEGER;

BEGIN
       IF Rst = '0' THEN
               --FIFO                                                  <= (OTHERS=> (OTHERS=>'0'));
               FIFO_TAIL_POINTER                       <= FifoSendBufferSize-1;
               FIFO_HEAD_POINTER                       <= 0;
               FIFO_ITEM_POPPED                        := FALSE;
               FIFO_WR_DATA1                           := (OTHERS=>'0');
               FIFO_EMPTY                                      <= TRUE;
               FIFO_FILL_LEVEL                         := 0;
               FIFO_WR_DATA1_EN                        := FALSE;

               SendBufferFull <= '0';
       ELSIF rising_edge(Clk) THEN
               --FIFO should initially do nothing                      
               FIFO_ITEM_POPPED := FALSE;
               FIFO_WR_DATA1_EN := FALSE;

               --Write to FIFO
               IF ProcessorToBuffer.DataAvailable = '1' THEN
                       FIFO_WR_DATA1 := ProcessorToBuffer.Address & ProcessorToBuffer.Data;
                       FIFO_WR_DATA1_EN := TRUE;               
               END IF;

               IF LocalStallSignal = '0' THEN
                       IF NOT FIFO_EMPTY THEN
                               FIFO_ITEM_POPPED := TRUE;               
                               BufferToNode.Address <= FIFO(FIFO_HEAD_POINTER)((Address_Length_X + Address_Length_Y) + Data_Length-1 DOWNTO Data_Length);
                               BufferToNode.Data <= FIFO(FIFO_HEAD_POINTER)(Data_Length-1 DOWNTO 0);   
                               BufferToNode.DataAvailable <= '1';
               
                       ELSE
                               BufferToNode.DataAvailable <= '0';
                               BufferToNode.Address <= (others => '0');
                               BufferToNode.Data <= (others => '0');
                       END IF;
                       
               END IF;

               --If item was removed from FIFO, move the Headpointer
               IF FIFO_ITEM_POPPED THEN
                       FIFO_HEAD_POINTER <= (FIFO_HEAD_POINTER + 1) MOD FifoSendBufferSize;                            
                       FIFO_FILL_LEVEL := FIFO_FILL_LEVEL-1;                           
               END IF;
                                               
                       
               FIFO_TAIL_TMP := FIFO_TAIL_POINTER;
                       
                       
               --Insert item(s) into FIFO                              
               IF FIFO_WR_DATA1_EN THEN
                       FIFO_TAIL_TMP := (FIFO_TAIL_TMP+1) MOD FifoSendBufferSize;
                       FIFO(FIFO_TAIL_TMP) <= FIFO_WR_DATA1;
                       FIFO_FILL_LEVEL := FIFO_FILL_LEVEL+1;                           
               END IF;
               
                       
                       
               --No change to the pointer if no data was written
               FIFO_TAIL_POINTER <= FIFO_TAIL_TMP;
                       
               --SendBufferFull detection                      
               IF FIFO_FILL_LEVEL >= FifoSendBufferSize THEN
                       SendBufferFull <= '1';
               END IF;
                       
               --FIFO empty?                   
               IF FIFO_FILL_LEVEL = 0 THEN
                       FIFO_EMPTY <= TRUE;                                     
               ELSE
                       FIFO_EMPTY <= FALSE;                                    
               END IF;         

       END IF; 
END PROCESS; 

END Behavioral;