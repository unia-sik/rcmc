LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;
USE IEEE.math_real.ALL;

entity NetworkInterfaceRecv IS
    PORT (
        Clk               : IN STD_LOGIC;
        Rst               : IN STD_LOGIC; 
        
        rdaddress : out  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        wraddress : out  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        
        NodeToBuffer      : IN P_PORT_BUFFER;
        BufferToProcessor           : out P_PORT_BUFFER;
        
        
        isEmpty         : OUT STD_LOGIC;
        rdclocken        : OUT STD_LOGIC;
        
        NodeClear  : IN STD_LOGIC
    );
END NetworkInterfaceRecv;

ARCHITECTURE Behavioral OF NetworkInterfaceRecv IS
 
SIGNAL counter : NATURAL := 0;
SIGNAL start_Pointer : NATURAL := 0;
SIGNAL next_Pointer : NATURAL := 0;
 
BEGIN

rdaddress <= STD_LOGIC_VECTOR(to_unsigned(start_Pointer, HeapSizeBits));
wraddress <= STD_LOGIC_VECTOR(to_unsigned(next_Pointer, HeapSizeBits));

process (clk)
    variable v_counter : natural;
begin
    if rising_edge(clk) then        
        v_counter := counter;
        BufferToProcessor <= NodeToBuffer;
        
        -- Delete if neccessary.
        IF NodeClear = '1' THEN
            if HeapSize = start_Pointer + 1 then
                start_Pointer <= 0;
            else
                start_Pointer <= start_Pointer + 1;
            end if;  
            v_counter := v_counter - 1;
        END IF;
                     
        --Handle Incoming
        IF NodeToBuffer.DataAvailable = '1' THEN                            
            if HeapSize = next_Pointer + 1 then
                next_Pointer <= 0;
            else
                next_Pointer <= next_Pointer + 1;
            end if;                   
            v_counter := v_counter + 1;
        END IF;
        
        if v_counter = 0 then
            isEmpty <= '1';
        else
            isEmpty <= '0';
        end if;
        
        if v_counter = 0 then
            rdclocken <= '0';
        else
            rdclocken <= '1';
        end if;
        
        counter <= v_counter;
    end if;
end process;
END Behavioral;

