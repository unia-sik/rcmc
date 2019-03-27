LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;
USE IEEE.math_real.ALL;

entity NetworkInterfaceSend IS
    PORT (
        Clk               : IN STD_LOGIC;
        Rst               : IN STD_LOGIC; 
        
        rdaddress : out  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        wraddress : out  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        
        ProcessorToBuffer      : IN P_PORT_BUFFER;
        BufferToNode           : out P_PORT_BUFFER;
        isFull         : OUT STD_LOGIC;
        rdclocken        : OUT STD_LOGIC;
        
        LocalRdyOut : OUT P_PORT_RDY;
        NodeClear  : IN STD_LOGIC;
        RdyClear  : IN STD_LOGIC
    );
END NetworkInterfaceSend;

ARCHITECTURE Behavioral OF NetworkInterfaceSend IS
 
SIGNAL RDY_Data : P_PORT_RDY;

SIGNAL counter : NATURAL := 0;
SIGNAL start_Pointer : NATURAL := 0;
SIGNAL next_Pointer : NATURAL := 0;
 
BEGIN

rdaddress <= STD_LOGIC_VECTOR(to_unsigned(start_Pointer, HeapSizeBits));
wraddress <= STD_LOGIC_VECTOR(to_unsigned(next_Pointer, HeapSizeBits));

process (clk)
    variable v_counter : natural;
    variable V_RDY_Data : P_PORT_RDY;
begin
    if rising_edge(clk) then        
        v_counter := counter;
        V_RDY_Data := RDY_Data;
        BufferToNode <= ProcessorToBuffer;
        
        -- Delete if neccessary.
        IF NodeClear = '1' THEN
            if HeapSize = start_Pointer + 1 then
                start_Pointer <= 0;
            else
                start_Pointer <= start_Pointer + 1;
            end if;  
            v_counter := v_counter - 1;
        END IF;
                
                        
        if RdyClear = '1' then
            V_RDY_Data.IsFirst := '0';
        end if;
        
        --Handle Incoming
        IF ProcessorToBuffer.DataAvailable = '1' THEN            
            if (ProcessorToBuffer.IsRdyInstr = '0') then                          
                if HeapSize = next_Pointer + 1 then
                    next_Pointer <= 0;
                else
                    next_Pointer <= next_Pointer + 1;
                end if;                   
                v_counter := v_counter + 1;
            else
                V_RDY_Data.Address.AddressReceiver := ProcessorToBuffer.Address;
                --sender will be set by the node
                V_RDY_Data.IsValid := '1';
                V_RDY_Data.IsFirst := '1';
            end if;
        END IF;
        
        if v_counter = 32 then
            isFull <= '1';
        else
            isFull <= '0';
        end if;
        
        if v_counter = 0 then
            rdclocken <= '0';
        else
            rdclocken <= '1';
        end if;
        
        counter <= v_counter;
        RDY_Data <= V_RDY_Data;
        
        LocalRdyOut <= V_RDY_Data;
    end if;
end process;
END Behavioral;
