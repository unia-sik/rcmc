LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;
USE ieee.math_real.floor;

ENTITY pnoo_node_no_rdy IS
    PORT (    
            Clk             : in std_logic;
            Rst             : in std_logic;            
            CoreAddress     : in  Address;
            
            NodeClear       : out std_logic;   
            
            NorthOut        : out P_PORT_VERTICAL;
            SouthIn         : in  P_PORT_VERTICAL;
            EastOut         : out P_PORT_HORIZONTAL;
            WestIn          : in  P_PORT_HORIZONTAL;
                                    
            LocalIn_1         : in  P_PORT_BUFFER;  
            LocalIn_2         : in  P_PORT_BUFFER;  
            LocalOut        : out P_PORT_BUFFER;
                                    
            cornerBufferOut_1  :out  P_PORT_HORIZONTAL;
            cornerBufferWriteAddr_1 :out std_logic_vector(HeapSizeBits - 1 downto 0);    
            cornerBufferWriteEnable_1 : out std_logic;        
            cornerBufferReadAddr_1 :out std_logic_vector(HeapSizeBits - 1 downto 0);             
            cornerBufferIn_1 :in P_PORT_HORIZONTAL;
            
            
            cornerBufferOut_2  :out P_PORT_HORIZONTAL;
            cornerBufferWriteAddr_2 :out std_logic_vector(HeapSizeBits - 1 downto 0);   
            cornerBufferWriteEnable_2 : out std_logic;         
            cornerBufferReadAddr_2 : out std_logic_vector(HeapSizeBits - 1 downto 0);             
            cornerBufferIn_2 : in P_PORT_HORIZONTAL
        );
END pnoo_node_no_rdy;


ARCHITECTURE Behavioral OF pnoo_node_no_rdy IS
    signal clkCounter : integer;
    signal phaseCounter : integer;

    signal cornerBufferSelector : integer;
    signal cornerBufferSelectorInv : integer;
    
    
    TYPE T_DATA_ARRAY_VECTOR IS ARRAY (0 TO 1) OF P_PORT_HORIZONTAL;    
    SIGNAL cornerBufferOut : T_DATA_ARRAY_VECTOR; 
    SIGNAL cornerBufferIn : T_DATA_ARRAY_VECTOR;
    
    TYPE T_ADDR_ARRAY_VECTOR IS ARRAY (0 TO 1) OF std_logic_vector(HeapSizeBits - 1 downto 0);    
    SIGNAL cornerBufferReadAddr : T_ADDR_ARRAY_VECTOR;
    SIGNAL cornerBufferWriteAddr : T_ADDR_ARRAY_VECTOR;
    
    TYPE T_ENABLE_ARRAY_VECTOR IS ARRAY (0 TO 1) OF std_logic;
    SIGNAL cornerBufferWriteEnable : T_ENABLE_ARRAY_VECTOR;
        
    function calc_distance (
        srcY : std_logic_vector(Address_Length_X-1 DOWNTO 0);
        destY : std_logic_vector(Address_Length_X-1 DOWNTO 0))
        return natural is
    begin
        if (to_integer(unsigned(destY)) + Dimension - to_integer(unsigned(srcY))) >= Dimension then
            return (to_integer(unsigned(destY)) + Dimension - to_integer(unsigned(srcY))) - Dimension;
        else
            return (to_integer(unsigned(destY)) + Dimension - to_integer(unsigned(srcY)));
        end if;
    end calc_distance;   
    
    function calc_core_id (
        A : Address)
        return natural is
    begin
            return (to_integer(unsigned(A.Y)) * Dimension + to_integer(unsigned(A.X)));
    end calc_core_id;    
BEGIN

    cornerBufferOut_1  <= cornerBufferOut(0);
    cornerBufferOut_2  <= cornerBufferOut(1);
    
    cornerBufferWriteAddr_1  <= cornerBufferWriteAddr(0);
    cornerBufferWriteAddr_2  <= cornerBufferWriteAddr(1);
    
    cornerBufferWriteEnable_1 <= cornerBufferWriteEnable(0);
    cornerBufferWriteEnable_2 <= cornerBufferWriteEnable(1);
    
    cornerBufferReadAddr_1  <= cornerBufferReadAddr(0);
    cornerBufferReadAddr_2  <= cornerBufferReadAddr(1);
    
    cornerBufferIn(0) <= cornerBufferIn_1;
    cornerBufferIn(1) <= cornerBufferIn_2;
    
    PROCESS (Clk, Rst)
        variable tmpInt : integer;
    BEGIN    
        IF RST = '0' THEN        
            --#######################################################################################################
            --# Reset signals
            --#######################################################################################################
            NorthOut.Data <= (others => '0');   
            NorthOut.Address.AddressReceiver.X <= (others => '0');   
            NorthOut.Address.AddressReceiver.Y <= (others => '0');   
            NorthOut.Address.AddressSender.X <= (others => '0');   
            NorthOut.Address.AddressSender.Y <= (others => '0');   
            NorthOut.DataAvailable <= '0';  
            NorthOut.IsRdyInstr <= '0'; 
            
            EastOut.Data <= (others => '0');   
            EastOut.Address.AddressReceiver.X <= (others => '0');   
            EastOut.Address.AddressReceiver.Y <= (others => '0');   
            EastOut.Address.AddressSender.X <= (others => '0');   
            EastOut.Address.AddressSender.Y <= (others => '0');   
            EastOut.DataAvailable <= '0';  
            EastOut.IsRdyInstr <= '0'; 
            
            clkCounter <= 0;
            phaseCounter <= 0;
            NodeClear <= '0';        
            cornerBufferSelector <= 0;
            cornerBufferSelectorInv <= 1;
            
            cornerBufferWriteEnable(0) <= '0';
            cornerBufferWriteEnable(1) <= '0';            
            cornerBufferOut(0) <= ((others => '0'), (((others => '0'), (others => '0')),((others => '0'), (others => '0'))), '0', '0');
            cornerBufferOut(1) <= ((others => '0'), (((others => '0'), (others => '0')),((others => '0'), (others => '0'))), '0', '0');            
            cornerBufferWriteAddr(0) <= (others => '0');
            cornerBufferWriteAddr(1) <= (others => '0');
            cornerBufferReadAddr(0) <= (others => '0');
            cornerBufferReadAddr(1) <= (others => '0');

            LocalOut.Data <= (others => '0');
            LocalOut.Address.X <= (others => '0');
            LocalOut.Address.Y <= (others => '0');
            LocalOut.DataAvailable  <= '0';
            LocalOut.IsRdyInstr <= '0';
        ELSIF rising_edge(CLK) THEN
            --#######################################################################################################
            --# Reset signals
            --#######################################################################################################
            NodeClear <= '0';

            LocalOut.Data <= (others => '0');
            LocalOut.Address.X <= (others => '0');
            LocalOut.Address.Y <= (others => '0');
            LocalOut.DataAvailable  <= '0';
            LocalOut.IsRdyInstr <= '0';
            
            NorthOut <= SouthIn;
            EastOut <= WestIn;             
            
            cornerBufferWriteEnable(0) <= '0';
            cornerBufferWriteEnable(1) <= '0';        
            cornerBufferOut(0) <= ((others => '0'), (((others => '0'), (others => '0')),((others => '0'), (others => '0'))), '0', '0');
            cornerBufferOut(1) <= ((others => '0'), (((others => '0'), (others => '0')),((others => '0'), (others => '0'))), '0', '0');            
            cornerBufferWriteAddr(0) <= (others => '0');
            cornerBufferWriteAddr(1) <= (others => '0');
            cornerBufferReadAddr(0) <= (others => '0');
            cornerBufferReadAddr(1) <= (others => '0');
                        
            
            --#######################################################################################################
            --# Incoming flits from South
            --#######################################################################################################
            
            if SouthIn.DataAvailable = '1' and 
               SouthIn.Address.AddressReceiver.X = CoreAddress.X and
               SouthIn.Address.AddressReceiver.Y = CoreAddress.Y 
            then
                LocalOut.Data          <= SouthIn.Data;    
                LocalOut.Address       <= SouthIn.Address.AddressSender;               
                LocalOut.DataAvailable <= '1';
                LocalOut.IsRdyInstr   <= '0';
                                
                NorthOut.Data                      <= (others => '0');   
                NorthOut.Address.AddressReceiver.X <= (others => '0');   
                NorthOut.Address.AddressReceiver.Y <= (others => '0');   
                NorthOut.Address.AddressSender.X   <= (others => '0');   
                NorthOut.Address.AddressSender.Y   <= (others => '0');   
                NorthOut.DataAvailable             <= '0';  
                NorthOut.IsRdyInstr               <= '0'; 
            end if; 
                       
            --#######################################################################################################
            --# Send next flit to North or to local Node
            --#######################################################################################################
                                  
            tmpInt := Dimension - 1 - phaseCounter;
            if tmpInt = calc_distance(CoreAddress.Y, cornerBufferIn(cornerBufferSelectorInv).Address.AddressReceiver.Y) then            
                if cornerBufferIn(cornerBufferSelectorInv).DataAvailable = '1' and cornerBufferIn(cornerBufferSelectorInv).Address.AddressReceiver.Y /= CoreAddress.Y then
                    NorthOut.Data          <= cornerBufferIn(cornerBufferSelectorInv).Data;
                    NorthOut.Address       <= cornerBufferIn(cornerBufferSelectorInv).Address;
                    NorthOut.DataAvailable <= cornerBufferIn(cornerBufferSelectorInv).DataAvailable;
                    NorthOut.IsRdyInstr    <= cornerBufferIn(cornerBufferSelectorInv).IsRdyInstr;
                    
                    cornerBufferWriteEnable(cornerBufferSelectorInv) <= '1';
                    cornerBufferOut(cornerBufferSelectorInv) <= ((others => '0'), (((others => '0'), (others => '0')),((others => '0'), (others => '0'))), '0', '0');
                    cornerBufferWriteAddr(cornerBufferSelectorInv) <= std_logic_vector(to_unsigned(tmpInt, 5));
                elsif cornerBufferIn(cornerBufferSelectorInv).DataAvailable = '1' and cornerBufferIn(cornerBufferSelectorInv).Address.AddressReceiver.Y = CoreAddress.Y then
                    LocalOut.Data          <= cornerBufferIn(cornerBufferSelectorInv).Data;    
                    LocalOut.Address       <= cornerBufferIn(cornerBufferSelectorInv).Address.AddressSender;                   
                    LocalOut.DataAvailable <= '1';
                    LocalOut.IsRdyInstr   <= '0';
                    
                    cornerBufferWriteEnable(cornerBufferSelectorInv) <= '1';
                    cornerBufferOut(cornerBufferSelectorInv) <= ((others => '0'), (((others => '0'), (others => '0')),((others => '0'), (others => '0'))), '0', '0');
                    cornerBufferWriteAddr(cornerBufferSelectorInv) <= std_logic_vector(to_unsigned(tmpInt, 5));
                end if;
            end if;
            
            --#######################################################################################################
            --# Insert into cornerBuffer
            --#######################################################################################################
            
            if WestIn.DataAvailable = '1' and WestIn.Address.AddressReceiver.X = CoreAddress.X then
                EastOut.Data                      <= (others => '0');   
                EastOut.Address.AddressReceiver.X <= (others => '0');   
                EastOut.Address.AddressReceiver.Y <= (others => '0');   
                EastOut.Address.AddressSender.X   <= (others => '0');   
                EastOut.Address.AddressSender.Y   <= (others => '0');   
                EastOut.DataAvailable             <= '0';  
                EastOut.IsRdyInstr               <= '0';

                cornerBufferWriteAddr(cornerBufferSelector) <= std_logic_vector(to_unsigned(calc_distance(CoreAddress.Y, WestIn.Address.AddressReceiver.Y), 5));
                cornerBufferWriteEnable(cornerBufferSelector) <= '1';
                cornerBufferOut(cornerBufferSelector) <= WestIn;

            end if;
                                  
            --#######################################################################################################
            --# Send data flit to East or insert into cornerBuffer
            --#######################################################################################################
                                    
            if phaseCounter = 0 and
               LocalIn_1.DataAvailable = '1' and
               LocalIn_1.IsRdyInstr = '0'
            then                
                if CoreAddress.X = LocalIn_1.Address.X then
                    cornerBufferWriteAddr(cornerBufferSelector) <= std_logic_vector(to_unsigned(calc_distance(CoreAddress.Y, LocalIn_1.Address.Y), 5));                    
                    cornerBufferWriteEnable(cornerBufferSelector) <= '1';
                    
                    cornerBufferOut(cornerBufferSelector).Data <= LocalIn_1.Data;
                    cornerBufferOut(cornerBufferSelector).Address.AddressReceiver <= LocalIn_1.Address;
                    cornerBufferOut(cornerBufferSelector).Address.AddressSender.X <= CoreAddress.X;
                    cornerBufferOut(cornerBufferSelector).Address.AddressSender.Y <= CoreAddress.Y;
                    cornerBufferOut(cornerBufferSelector).DataAvailable <= '1';
                    cornerBufferOut(cornerBufferSelector).IsRdyInstr <= '0';
                else
                    EastOut.Data <= LocalIn_1.Data;
                    EastOut.Address.AddressReceiver <= LocalIn_1.Address;
                    EastOut.Address.AddressSender.X <= CoreAddress.X;
                    EastOut.Address.AddressSender.Y <= CoreAddress.Y;
                    EastOut.DataAvailable <= '1';
                    EastOut.IsRdyInstr <= '0';
                end if;
                
                NodeClear <= '1';
            elsif phaseCounter = 0 and
               LocalIn_2.DataAvailable = '1' and
               LocalIn_2.IsRdyInstr = '0' 
            then                
                if CoreAddress.X = LocalIn_2.Address.X then
                    cornerBufferWriteAddr(cornerBufferSelector) <= std_logic_vector(to_unsigned(calc_distance(CoreAddress.Y, LocalIn_2.Address.Y), 5));                  
                    cornerBufferWriteEnable(cornerBufferSelector) <= '1';
                    
                    cornerBufferOut(cornerBufferSelector).Data <= LocalIn_2.Data;
                    cornerBufferOut(cornerBufferSelector).Address.AddressReceiver <= LocalIn_2.Address;
                    cornerBufferOut(cornerBufferSelector).Address.AddressSender.X <= CoreAddress.X;
                    cornerBufferOut(cornerBufferSelector).Address.AddressSender.Y <= CoreAddress.Y;
                    cornerBufferOut(cornerBufferSelector).DataAvailable <= '1';
                    cornerBufferOut(cornerBufferSelector).IsRdyInstr <= '0';
                else
                    EastOut.Data <= LocalIn_2.Data;
                    EastOut.Address.AddressReceiver <= LocalIn_2.Address;
                    EastOut.Address.AddressSender.X <= CoreAddress.X;
                    EastOut.Address.AddressSender.Y <= CoreAddress.Y;
                    EastOut.DataAvailable <= '1';
                    EastOut.IsRdyInstr <= '0';
                end if;
                
                NodeClear <= '1';
            end if;
              
            --#######################################################################################################
            --# Update local counters and arrays
            --#######################################################################################################
                                    
            clkCounter <= clkCounter + 1;
            phaseCounter <= phaseCounter + 1;
            
            if phaseCounter + 1 = Dimension then
                phaseCounter <= 0;
                cornerBufferSelector <= cornerBufferSelectorInv;
                cornerBufferSelectorInv <= cornerBufferSelector;                
                
                cornerBufferReadAddr(0) <= std_logic_vector(to_unsigned(Dimension - 1, 5));
                cornerBufferReadAddr(1) <= std_logic_vector(to_unsigned(Dimension - 1, 5));
            else
                cornerBufferSelector <= cornerBufferSelector;
                cornerBufferSelectorInv <= cornerBufferSelectorInv;
                
                cornerBufferReadAddr(0) <= std_logic_vector(to_unsigned(Dimension - 1 - phaseCounter - 1, 5));
                cornerBufferReadAddr(1) <= std_logic_vector(to_unsigned(Dimension - 1 - phaseCounter - 1, 5));
            end if;            
        END IF;

    END PROCESS;
    
END Behavioral;
