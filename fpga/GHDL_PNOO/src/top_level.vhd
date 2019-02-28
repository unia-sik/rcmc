LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE WORK.CONSTANTS.ALL;
use ieee.numeric_std.all;
USE WORK.LIBNODE.ALL;


ENTITY top_level IS
  PORT (
            --KEY        : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
            --CLOCK_50    : IN STD_LOGIC;
            LEDR        : OUT STD_LOGIC_VECTOR((cntNoJoke - 1) DOWNTO 0));
END;

ARCHITECTURE STRUCTURE OF top_level IS

component NOCUNIT
generic (id : integer; count : integer; nocdim : STD_LOGIC_VECTOR(63 downto 0));
PORT (    
    Clk                : IN std_logic;
    rst_n              : IN std_logic;                
    LED                : out std_logic;
    NorthOut           : OUT P_PORT_VERTICAL;
    SouthIn            : IN  P_PORT_VERTICAL;
    EastOut            : OUT P_PORT_HORIZONTAL;
    WestIn             : IN  P_PORT_HORIZONTAL;                            
        
    RdyIn_1           : IN P_PORT_RDY;
    RdyOut_1          : OUT P_PORT_RDY;
        
    RdyIn_2           : IN P_PORT_RDY;
    RdyOut_2          : OUT P_PORT_RDY;
    
    CoreAddress            : IN  Address                    
    );
END component;

component sim_clock
    port(clk : out std_logic);
end component;

component sim_reset
    port(rst_n : out std_logic);
end component;

SIGNAL CLOCK_50 : std_logic;
SIGNAL RST_N : std_logic;

TYPE VECTOR_ADDRESS is array (natural range <>) of Address;
SIGNAL N_XX_ADDRESS : VECTOR_ADDRESS((cntNoJoke - 1) downto 0);

TYPE VECTOR_PORT_VERTICAL is array (natural range <>) of P_PORT_VERTICAL;
SIGNAL N_XX_NORTH_OUT : VECTOR_PORT_VERTICAL((cntNoJoke - 1) downto 0);

TYPE VECTOR_PORT_HORIZONTAL is array (natural range <>) of P_PORT_HORIZONTAL;
SIGNAL N_XX_EAST_OUT : VECTOR_PORT_HORIZONTAL((cntNoJoke - 1) downto 0);

TYPE VECTOR_PORT_RDY is array (natural range <>) of P_PORT_RDY;
SIGNAL N_XX_RDY_OUT_1 : VECTOR_PORT_RDY((cntNoJoke - 1) downto 0);
SIGNAL N_XX_RDY_IN_1 : VECTOR_PORT_RDY((cntNoJoke - 1) downto 0);

SIGNAL N_XX_RDY_OUT_2 : VECTOR_PORT_RDY((cntNoJoke - 1) downto 0);
SIGNAL N_XX_RDY_IN_2 : VECTOR_PORT_RDY((cntNoJoke - 1) downto 0);

begin

CLOCK : sim_clock
port map(
    clk => CLOCK_50
);

RESET : sim_reset
port map(
    rst_n => RST_N
);

-- nodeCluster: 
NODE_Y: for y in 0 to (dimNoJoke - 1) generate		--(dimNoJoke - 1)
    NODE_X: for x in 0 to (dimNoJoke - 1) generate
        
    NOCUNIT_INSTANCE : NOCUNIT
    generic map (
        id => to_integer(unsigned(x"00000000" & std_logic_vector(to_unsigned(y, 16)) & std_logic_vector(to_unsigned(x, 16)))), 
        count => cntNoJoke, 
        nocdim => x"00010001" & std_logic_vector(to_unsigned(dimNoJoke, 16)) & std_logic_vector(to_unsigned(dimNoJoke, 16))
    )
    port map( 
            Clk   => CLOCK_50,
            rst_n => RST_N,
            LED   => LEDR(y * dimNoJoke + x),
            
            NorthOut => N_XX_NORTH_OUT(y * dimNoJoke + x),  
            SouthIn  => N_XX_NORTH_OUT(((y + dimNoJoke - 1) mod dimNoJoke) * dimNoJoke + x),  
            EastOut  => N_XX_EAST_OUT(y * dimNoJoke + x),
            WestIn   => N_XX_EAST_OUT(y * dimNoJoke + ((x + dimNoJoke - 1) mod dimNoJoke)),            
            
            rdyIn_1  => N_XX_RDY_IN_1(y * dimNoJoke + x),
            rdyOut_1 => N_XX_RDY_OUT_1(y * dimNoJoke + x),
            
            rdyIn_2  => N_XX_RDY_IN_2(y * dimNoJoke + x),
            rdyOut_2 => N_XX_RDY_OUT_2(y * dimNoJoke + x),
            
            CoreAddress => N_XX_ADDRESS(y * dimNoJoke + x)
    );   
    
    N_XX_ADDRESS(y * dimNoJoke + x).x <= std_logic_vector(to_unsigned(x, Address_Length_X));
    N_XX_ADDRESS(y * dimNoJoke + x).y <= std_logic_vector(to_unsigned(y, Address_Length_Y));    
    
--     RDY_NETWORK_START: if (y * dimNoJoke + x) = 0 generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(dimNoJoke * dimNoJoke - 1);
--         N_XX_RDY_IN_2(dimNoJoke * dimNoJoke - 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);    
--     end generate RDY_NETWORK_START;
--     
--     RDY_NETWORK_RUN: if 0 < (y * dimNoJoke + x) generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x - 1);
--         N_XX_RDY_IN_2(y * dimNoJoke + x - 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);    
--     end generate RDY_NETWORK_RUN;
    
    
    
--     RDY_NETWORK_START_EVEN: if x = 0 and y /= (dimNoJoke - 1) generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1((y + 1) * dimNoJoke + x);
--         N_XX_RDY_IN_2((y + 1) * dimNoJoke + x) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
--     end generate RDY_NETWORK_START_EVEN;
--     
--     RDY_NETWORK_LEFT_TO_RIGHT_NORMAL_BOT: if y = 0 and x /= 0 generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x - 1);
--         N_XX_RDY_IN_2(y * dimNoJoke + x - 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
--     end generate RDY_NETWORK_LEFT_TO_RIGHT_NORMAL_BOT;
--     
--     RDY_NETWORK_LEFT_TO_RIGHT_NORMAL: if (y mod 2) = 0 and y > 0 and x > 1 generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x - 1);
--         N_XX_RDY_IN_2(y * dimNoJoke + x - 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
--     end generate RDY_NETWORK_LEFT_TO_RIGHT_NORMAL;
--     
--     RDY_NETWORK_LEFT_TO_RIGHT_BORDER: if (y mod 2) = 0 and x = 1 and y > 0 generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x - dimNoJoke);
--         N_XX_RDY_IN_2(y * dimNoJoke + x - dimNoJoke) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
--     end generate RDY_NETWORK_LEFT_TO_RIGHT_BORDER;
--     
--     
--     RDY_NETWORK_RIGHT_TO_LEFT_TOP: if (y mod 2) = 1 and x /= (dimNoJoke - 1) and y = (dimNoJoke - 1) generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x + 1);
--         N_XX_RDY_IN_2(y * dimNoJoke + x + 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
--     end generate RDY_NETWORK_RIGHT_TO_LEFT_TOP;
--        
--     
--     RDY_NETWORK_RIGHT_TO_LEFT_NORMAL: if (y mod 2) = 1 and x /= 0 and y /= (dimNoJoke - 1) and x /= (dimNoJoke - 1) generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x + 1);
--         N_XX_RDY_IN_2(y * dimNoJoke + x + 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
--     end generate RDY_NETWORK_RIGHT_TO_LEFT_NORMAL;
--     
--     RDY_NETWORK_RIGHT_TO_LEFT_BORDER: if (y mod 2) = 1 and x = (dimNoJoke - 1) generate
--         N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x - dimNoJoke);
--         N_XX_RDY_IN_2(y * dimNoJoke + x - dimNoJoke) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
--     end generate RDY_NETWORK_RIGHT_TO_LEFT_BORDER;
     
    
    
    
    
    RDY_NETWORK_START_EVEN: if x = 0 and y = 0 and (dimNoJoke mod 2) = 0 generate
        N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(dimNoJoke * dimNoJoke - 1 - dimNoJoke + 1);
        N_XX_RDY_IN_2(dimNoJoke * dimNoJoke - 1 - dimNoJoke + 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
    end generate RDY_NETWORK_START_EVEN;
    
    RDY_NETWORK_START_ODD: if x = 0 and y = 0 and (dimNoJoke mod 2) = 1 generate
        N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(dimNoJoke * dimNoJoke - 1);
        N_XX_RDY_IN_2(dimNoJoke * dimNoJoke - 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
    end generate RDY_NETWORK_START_ODD;
    
    RDY_NETWORK_LEFT_TO_RIGHT_NORMAL: if (y mod 2) = 0 and x /= 0 generate
        N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x - 1);
        N_XX_RDY_IN_2(y * dimNoJoke + x - 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
    end generate RDY_NETWORK_LEFT_TO_RIGHT_NORMAL;
    
    RDY_NETWORK_LEFT_TO_RIGHT_BORDER: if (y mod 2) = 0 and x = 0 and y > 0 generate
        N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x - dimNoJoke);
        N_XX_RDY_IN_2(y * dimNoJoke + x - dimNoJoke) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
    end generate RDY_NETWORK_LEFT_TO_RIGHT_BORDER;
    
    RDY_NETWORK_RIGHT_TO_LEFT_NORMAL: if (y mod 2) = 1 and x /= (dimNoJoke - 1) generate
        N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x + 1);
        N_XX_RDY_IN_2(y * dimNoJoke + x + 1) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
    end generate RDY_NETWORK_RIGHT_TO_LEFT_NORMAL;
    
    RDY_NETWORK_RIGHT_TO_LEFT_BORDER: if (y mod 2) = 1 and x = (dimNoJoke - 1) generate
        N_XX_RDY_IN_1(y * dimNoJoke + x) <= N_XX_RDY_OUT_1(y * dimNoJoke + x - dimNoJoke);
        N_XX_RDY_IN_2(y * dimNoJoke + x - dimNoJoke) <= N_XX_RDY_OUT_2(y * dimNoJoke + x);
    end generate RDY_NETWORK_RIGHT_TO_LEFT_BORDER;
    
    end generate NODE_X;
end generate NODE_Y;
-- end generate 


end; 

