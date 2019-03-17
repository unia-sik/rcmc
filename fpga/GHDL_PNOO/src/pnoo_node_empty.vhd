LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;
USE ieee.math_real.floor;

ENTITY pnoo_node_empty IS
    PORT (    
            Clk             : in std_logic;
            Rst             : in std_logic
        );
END pnoo_node_empty;


ARCHITECTURE Behavioral OF pnoo_node_empty IS
BEGIN
    PROCESS (Clk, Rst)
    BEGIN    

    END PROCESS;
    
END Behavioral;

