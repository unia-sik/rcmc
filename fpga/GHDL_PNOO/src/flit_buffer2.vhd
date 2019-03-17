LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.numeric_std.all;
USE work.constants.ALL;
USE work.LibNode.ALL;

ENTITY flit_buffer2 IS 
    PORT(        
        rdaddress : IN  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        rdclock   : IN  STD_LOGIC;
        rdclocken : IN  STD_LOGIC := '0';
        
        data      : IN  P_PORT_HORIZONTAL;
        wraddress : IN  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        wrclock   : IN  STD_LOGIC := '1';
        wrclocken : IN  STD_LOGIC := '1';
        wren      : IN  STD_LOGIC := '0';
        
        q         : OUT P_PORT_HORIZONTAL
    );
END flit_buffer2;

ARCHITECTURE SYN OF flit_buffer2 IS
    TYPE T_DATA_ARRAY IS ARRAY (0 TO Dimension - 1) OF P_PORT_HORIZONTAL; --HeapSize
    SIGNAL Data_Array : T_DATA_ARRAY := (others => 
                                            ((others => '0'), (((others => '0'), (others => '0')),((others => '0'), (others => '0'))), '0', '0')
                                        );

BEGIN
    process(rdclocken, rdaddress, wraddress, wren)
    begin
--          if rising_edge(rdclock) then
          if rdclocken = '1' then
            if (to_integer(unsigned(rdaddress)) /= to_integer(unsigned(wraddress))) or wren = '0' then
                q <= Data_Array(to_integer(unsigned(rdaddress)));
            else
                q <= data;
            end if;
          else
                q <= ((others => '0'), (((others => '0'), (others => '0')),((others => '0'), (others => '0'))), '0', '0');
          end if;
--          end if;
    end process;

    process(wrclock)
    begin
        if rising_edge(wrclock) then
            if wrclocken = '1' then
                if wren = '1' then
--                    if then
                        Data_Array(to_integer(unsigned(wraddress))) <= data;
  --                  end if;
                end if;
            end if;
        end if;
    end process;
end SYN;



