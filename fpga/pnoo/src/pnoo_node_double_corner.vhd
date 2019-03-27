
LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;
USE ieee.math_real.floor;

ENTITY pnoo_node_double_corner IS
    PORT (    
            Clk             : in std_logic;
            Rst             : in std_logic;            
            CoreAddress     : in  Address;
            
            NodeClear       : out std_logic;   
            RdyClear        : out std_logic;   
            RecvClear       : in std_logic;
            
            NorthOut        : out P_PORT_VERTICAL;
            SouthIn         : in  P_PORT_VERTICAL;
            EastOut         : out P_PORT_HORIZONTAL;
            WestIn          : in  P_PORT_HORIZONTAL;
            
            RdyIn_1         : in P_PORT_RDY;
            RdyIn_2         : in P_PORT_RDY;
            
            RdyOut_1        : out P_PORT_RDY;           
            RdyOut_2        : out P_PORT_RDY;
                        
            LocalIn_1         : in  P_PORT_BUFFER;  
            LocalIn_2         : in  P_PORT_BUFFER;  
            LocalOut        : out P_PORT_BUFFER;
                        
            LocalRdyIn_1    : out RdyAddress;
            LocalRdyIn_2    : out RdyAddress;
            LocalRdyOut     : in  P_PORT_RDY;
            
            BarrierConfigIn : in BarrierConfig;
            BarrierSetOut   : out std_logic;
            
            
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
END pnoo_node_double_corner;


ARCHITECTURE Behavioral OF pnoo_node_double_corner IS
    signal clkCounter : integer;
    signal phaseCounter : integer;
    signal srdyPhaseCounter : integer;
    
    signal barrierCounter : Address;
    signal barrierStartAddr : Address;
    signal barrierEndAddr : Address;
    signal barrierSet : std_logic;
    
    signal recvSlots : integer;

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
           
    TYPE T_RDY_ARRIVED_BUF_DATA IS ARRAY(0 to Dimension - 1) of bit; 
    TYPE T_RDY_ARRIVED_BUF_VEC IS ARRAY(0 to Dimension - 1) of T_RDY_ARRIVED_BUF_DATA; 
    SIGNAL RDY_ARRIVED_BUF : T_RDY_ARRIVED_BUF_VEC;
    SIGNAL RDY_ARRIVED_BUF_CHANGE : T_RDY_ARRIVED_BUF_VEC;
    
    CONSTANT srdyPhaseCounterLimit : integer := integer(floor(REAL(Dimension * Dimension) / 2.0));
    
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
        variable recvSlots_V : integer;
        variable barrierCounter_V : Address;
        variable barrierStartAddr_V : Address;
        variable barrierEndAddr_V : Address;
        variable barrierSet_V : std_logic;
        variable RDY_ARRIVED_BUF_VAR: T_RDY_ARRIVED_BUF_VEC;
        variable RDY_ARRIVED_BUF_CHANGE_VAR: T_RDY_ARRIVED_BUF_VEC;
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
            srdyPhaseCounter <= 0;
            
            barrierCounter.X <= (others => '0');
            barrierCounter.Y <= (others => '0');
            barrierStartAddr.X <= (others => '0');
            barrierStartAddr.Y <= (others => '0');
            barrierEndAddr.X <= (others => '0');
            barrierEndAddr.Y <= (others => '0');
            BarrierSetOut <= '0';
            barrierSet <= '0';
            
            recvSlots <= 0;
            NodeClear <= '0';   
            RdyClear <= '0';        
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
            
            RdyOut_1.Address.AddressReceiver.X <= (others => '0');
            RdyOut_1.Address.AddressReceiver.Y <= (others => '0');
            RdyOut_1.Address.AddressSender.X <= (others => '0');
            RdyOut_1.Address.AddressSender.Y <= (others => '0');
            RdyOut_1.IsValid  <= '0';
            RdyOut_1.IsFirst <= '0';
            RdyOut_1.IsBarrier <= '0';
            
            RdyOut_2.Address.AddressReceiver.X <= (others => '0');
            RdyOut_2.Address.AddressReceiver.Y <= (others => '0');
            RdyOut_2.Address.AddressSender.X <= (others => '0');
            RdyOut_2.Address.AddressSender.Y <= (others => '0');
            RdyOut_2.IsValid  <= '0';
            RdyOut_2.IsFirst <= '0';
            RdyOut_2.IsBarrier <= '0';

            LocalOut.Data <= (others => '0');
            LocalOut.Address.X <= (others => '0');
            LocalOut.Address.Y <= (others => '0');
            LocalOut.DataAvailable  <= '0';
            LocalOut.IsRdyInstr <= '0';
            
            LocalRdyIn_1.X <= (others => '0');
            LocalRdyIn_1.Y <= (others => '0');
            LocalRdyIn_1.Toogle <= '0';
            
            LocalRdyIn_2.X <= (others => '0');
            LocalRdyIn_2.Y <= (others => '0');
            LocalRdyIn_2.Toogle <= '0';
        ELSIF rising_edge(CLK) THEN
            --#######################################################################################################
            --# Reset signals
            --#######################################################################################################
            NodeClear <= '0';
            RdyClear <= '0';
            
            RdyOut_1.Address.AddressReceiver.X <= (others => '0');
            RdyOut_1.Address.AddressReceiver.Y <= (others => '0');
            RdyOut_1.Address.AddressSender.X <= (others => '0');
            RdyOut_1.Address.AddressSender.Y <= (others => '0');
            RdyOut_1.IsValid  <= '0';
            RdyOut_1.IsFirst <= '0';
            RdyOut_1.IsBarrier <= '0';
            
            
            RdyOut_2.Address.AddressReceiver.X <= (others => '0');
            RdyOut_2.Address.AddressReceiver.Y <= (others => '0');
            RdyOut_2.Address.AddressSender.X <= (others => '0');
            RdyOut_2.Address.AddressSender.Y <= (others => '0');
            RdyOut_2.IsValid  <= '0';
            RdyOut_2.IsFirst <= '0';
            RdyOut_2.IsBarrier <= '0';

            LocalOut.Data <= (others => '0');
            LocalOut.Address.X <= (others => '0');
            LocalOut.Address.Y <= (others => '0');
            LocalOut.DataAvailable  <= '0';
            LocalOut.IsRdyInstr <= '0';
            
            LocalRdyIn_1.X <= (others => '0');
            LocalRdyIn_1.Y <= (others => '0');
            LocalRdyIn_1.Toogle <= '0';
            
            LocalRdyIn_2.X <= (others => '0');
            LocalRdyIn_2.Y <= (others => '0');
            LocalRdyIn_2.Toogle <= '0';

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
                    
            barrierCounter_V := barrierCounter;
            barrierStartAddr_V := barrierStartAddr;
            barrierEndAddr_V := barrierEndAddr;
            barrierSet_V     := barrierSet;
            recvSlots_V := recvSlots;
            RDY_ARRIVED_BUF_VAR := Rdy_ARRIVED_BUF;
            RDY_ARRIVED_BUF_CHANGE_VAR := Rdy_ARRIVED_BUF_CHANGE;
            
            --#######################################################################################################
            --# Set up barrier
            --#######################################################################################################
            if BarrierConfigIn.Set = '1' then
                barrierStartAddr_V := BarrierConfigIn.StartAddr;
                barrierEndAddr_V   := BarrierConfigIn.EndAddr;
                barrierCounter_V := BarrierConfigIn.StartAddr;
                barrierSet_V     := '0';
            end if;
            
            
            
            barrierSet <= barrierSet;
            
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
                
                recvSlots_V := recvSlots_V + 1;
                
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
                    
                    recvSlots_V := recvSlots_V + 1;
                    
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
                                
-- --                 if phaseCounter = Dimension - 1 then
-- --                     cornerBuffer(cornerBufferSelectorInv)(calc_distance(CoreAddress.Y, WestIn.Address.AddressReceiver.Y)) <= WestIn;
-- --                 else
                cornerBufferWriteAddr(cornerBufferSelector) <= std_logic_vector(to_unsigned(calc_distance(CoreAddress.Y, WestIn.Address.AddressReceiver.Y), 5));
                cornerBufferWriteEnable(cornerBufferSelector) <= '1';
                cornerBufferOut(cornerBufferSelector) <= WestIn;
-- --                 end if;
            end if;
            
            --#######################################################################################################
            --# Receive srdy from Ring-1
            --#######################################################################################################            
                    
            if RdyIn_1.IsValid = '1' then
                if RdyIn_1.Address.AddressReceiver = CoreAddress then  
                    if RdyIn_1.IsFirst = '1' then
                        LocalRdyIn_1.X <= RdyIn_1.Address.AddressSender.X;
                        LocalRdyIn_1.Y <= RdyIn_1.Address.AddressSender.Y;
                        LocalRdyIn_1.Toogle <= '1';  
                    end if;
                    
                    
                    
                    if (RDY_ARRIVED_BUF_VAR(to_integer(unsigned(RdyIn_1.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_1.Address.AddressSender.X))) = '0') then
                        RDY_ARRIVED_BUF_VAR(to_integer(unsigned(RdyIn_1.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_1.Address.AddressSender.X))) := '1'; 
                        RDY_ARRIVED_BUF_CHANGE_VAR(to_integer(unsigned(RdyIn_1.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_1.Address.AddressSender.X))) := '1'; 
                    end if;
                else
                    LocalRdyIn_1.X <= (others => '0');
                    LocalRdyIn_1.Y <= (others => '0');
                    LocalRdyIn_1.Toogle <= '0';                
                    
                    if (RDY_ARRIVED_BUF_VAR(to_integer(unsigned(RdyIn_1.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_1.Address.AddressSender.X))) = '1') then
                        RDY_ARRIVED_BUF_VAR(to_integer(unsigned(RdyIn_1.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_1.Address.AddressSender.X))) := '0'; 
                        RDY_ARRIVED_BUF_CHANGE_VAR(to_integer(unsigned(RdyIn_1.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_1.Address.AddressSender.X))) := '1'; 
                    end if;
                end if;
                
                if RdyIn_1.IsBarrier = '1' then
                    if barrierStartAddr_V.X <= RdyIn_1.Address.AddressSender.X and RdyIn_1.Address.AddressSender.X <= barrierEndAddr_V.X and
                       barrierStartAddr_V.Y <= RdyIn_1.Address.AddressSender.Y and RdyIn_1.Address.AddressSender.Y <= barrierEndAddr_V.Y 
                    then                        
                        if barrierCounter_V.X /= barrierEndAddr_V.X then
                            barrierCounter_V.X := std_logic_vector(to_unsigned(to_integer(unsigned(barrierCounter_V.X) + 1), Address_Length_X));
                        elsif barrierCounter_V.Y /= barrierEndAddr_V.Y then
                            barrierCounter_V.X := barrierStartAddr_V.X;                            
                            barrierCounter_V.Y := std_logic_vector(to_unsigned(to_integer(unsigned(barrierCounter_V.Y) + 1), Address_Length_Y));
                        else
                            barrierSet_V := '1';
                        end if; 
                    end if;
                end if;
            end if;
            
            --#######################################################################################################
            --# Receive srdy from Ring-2
            --#######################################################################################################
            
            if RdyIn_2.IsValid = '1' then
                if RdyIn_2.Address.AddressReceiver = CoreAddress then  
                    if RdyIn_2.IsFirst = '1' then
                        LocalRdyIn_2.X <= RdyIn_2.Address.AddressSender.X;
                        LocalRdyIn_2.Y <= RdyIn_2.Address.AddressSender.Y;
                        LocalRdyIn_2.Toogle <= '1';  
                    end if;
                    
--                     
                    if (RDY_ARRIVED_BUF_VAR(to_integer(unsigned(RdyIn_2.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_2.Address.AddressSender.X))) = '0') then
                        RDY_ARRIVED_BUF_VAR(to_integer(unsigned(RdyIn_2.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_2.Address.AddressSender.X))) := '1'; 
                        RDY_ARRIVED_BUF_CHANGE_VAR(to_integer(unsigned(RdyIn_2.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_2.Address.AddressSender.X))) := '1'; 
                    end if;
                else
                    LocalRdyIn_2.X <= (others => '0');
                    LocalRdyIn_2.Y <= (others => '0');
                    LocalRdyIn_2.Toogle <= '0';                
                    
                    if (RDY_ARRIVED_BUF_VAR(to_integer(unsigned(RdyIn_2.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_2.Address.AddressSender.X))) = '1') then
                        RDY_ARRIVED_BUF_VAR(to_integer(unsigned(RdyIn_2.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_2.Address.AddressSender.X))) := '0'; 
                        RDY_ARRIVED_BUF_CHANGE_VAR(to_integer(unsigned(RdyIn_2.Address.AddressSender.Y)))(to_integer(unsigned(RdyIn_2.Address.AddressSender.X))) := '1'; 
                    end if;
                end if;
                
                if RdyIn_2.IsBarrier = '1' then
                    if barrierStartAddr_V.X <= RdyIn_2.Address.AddressSender.X and RdyIn_2.Address.AddressSender.X <= barrierEndAddr_V.X and
                       barrierStartAddr_V.Y <= RdyIn_2.Address.AddressSender.Y and RdyIn_2.Address.AddressSender.Y <= barrierEndAddr_V.Y and
                       RdyIn_2.Address.AddressSender /= RdyIn_1.Address.AddressSender
                    then
                        if barrierCounter_V.X /= barrierEndAddr_V.X then
                            barrierCounter_V.X := std_logic_vector(to_unsigned(to_integer(unsigned(barrierCounter_V.X) + 1), Address_Length_X));
                        elsif barrierCounter_V.Y /= barrierEndAddr_V.Y then
                            barrierCounter_V.X := barrierStartAddr_V.X;                            
                            barrierCounter_V.Y := std_logic_vector(to_unsigned(to_integer(unsigned(barrierCounter_V.Y) + 1), Address_Length_Y));
                        else
                            barrierSet_V := '1';
                        end if; 
                    end if;
                end if;
            end if;
            
            --#######################################################################################################
            --# Update srdy-release array and forward Ring-1 and Ring-2
            --#######################################################################################################
                        
            if srdyPhaseCounter = 0 then
                for i in 0 to Dimension - 1 loop
                    for j in 0 to Dimension - 1 loop
                        RDY_ARRIVED_BUF_CHANGE_VAR(i)(j) := '0';
                    end loop;
                end loop;
            else
                RdyOut_1 <= RdyIn_1;
                RdyOut_2 <= RdyIn_2;
            end if;
            
            --#######################################################################################################
            --# Update the recv-counter values
            --#######################################################################################################
            
             if RecvClear = '1' then
                recvSlots_V := recvSlots_V - 1;
            end if;
            
            --#######################################################################################################
            --# Send srdy-flits on Ring-1 and Ring-2
            --#######################################################################################################
            
             if srdyPhaseCounter = 0 then        
                if recvSlots_V < (HeapSize - Dimension - 1) and LocalRdyOut.IsValid = '1' then
                    RdyOut_1.Address.AddressReceiver <= LocalRdyOut.Address.AddressReceiver;
                    RdyOut_1.Address.AddressSender   <= CoreAddress;
                    RdyOut_1.IsValid                 <= '1';
                    RdyOut_1.IsFirst                 <= LocalRdyOut.IsFirst;
                    
                    RdyOut_2.Address.AddressReceiver <= LocalRdyOut.Address.AddressReceiver;
                    RdyOut_2.Address.AddressSender   <= CoreAddress;
                    RdyOut_2.IsValid                 <= '1';
                    RdyOut_2.IsFirst                 <= LocalRdyOut.IsFirst;
                    
                    if LocalRdyOut.IsFirst = '1' then
                        RdyClear <= '1';                    
                    end if;
                else
                    RdyOut_1.Address.AddressReceiver <= CoreAddress;
                    RdyOut_1.Address.AddressSender   <= CoreAddress;
                    RdyOut_1.IsValid                 <= '1';
                    RdyOut_1.IsFirst                 <= '0';                
                    
                    RdyOut_2.Address.AddressReceiver <= CoreAddress;
                    RdyOut_2.Address.AddressSender   <= CoreAddress;
                    RdyOut_2.IsValid                 <= '1';
                    RdyOut_2.IsFirst                 <= '0';                
                end if;    
                    
                --reset at the beginning
                barrierCounter_V := barrierStartAddr_V;
                RdyOut_1.IsBarrier <= '0';
                RdyOut_2.IsBarrier <= '0';
                
                if barrierSet_V = '0' then
                    
                    if barrierStartAddr_V.X /= barrierEndAddr_V.X or barrierStartAddr_V.Y /= barrierEndAddr_V.Y then
                        RdyOut_1.IsBarrier <= '1';
                        RdyOut_2.IsBarrier <= '1';
                    
                        if barrierCounter_V.X /= barrierEndAddr_V.X then
                            barrierCounter_V.X := std_logic_vector(to_unsigned(1, Address_Length_X));
                        elsif barrierCounter_V.Y /= barrierEndAddr_V.Y then
                            barrierCounter_V.X := barrierStartAddr_V.X;                            
                            barrierCounter_V.Y := std_logic_vector(to_unsigned(1, Address_Length_Y));
                        end if;
                    
                    
--                         barrierCounter_V.X := std_logic_vector(to_unsigned(1, Address_Length_X));
                    end if;
                end if;                          
            end if; 
            
            --#######################################################################################################
            --# Send data flit to East or insert into cornerBuffer
            --#######################################################################################################
                                    
            if phaseCounter = 0 and
               LocalIn_1.DataAvailable = '1' and
               LocalIn_1.IsRdyInstr = '0' and               
               (
                    RDY_ARRIVED_BUF_VAR(to_integer(unsigned(LocalIn_1.Address.Y)))(to_integer(unsigned(LocalIn_1.Address.X))) = '1' xor
                    Rdy_ARRIVED_BUF_CHANGE_VAR(to_integer(unsigned(LocalIn_1.Address.Y)))(to_integer(unsigned(LocalIn_1.Address.X))) = '1'
               ) 
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
               LocalIn_2.IsRdyInstr = '0' and           
               (
                    RDY_ARRIVED_BUF_VAR(to_integer(unsigned(LocalIn_2.Address.Y)))(to_integer(unsigned(LocalIn_2.Address.X))) = '1' xor
                    Rdy_ARRIVED_BUF_CHANGE_VAR(to_integer(unsigned(LocalIn_2.Address.Y)))(to_integer(unsigned(LocalIn_2.Address.X))) = '1'
               ) 
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
              
            barrierCounter <= barrierCounter_V;
            barrierStartAddr <= barrierStartAddr_V;
            barrierEndAddr <= barrierEndAddr_V;
            barrierSet     <= barrierSet_V;
            BarrierSetOut <= barrierSet_V;
            
            recvSlots <= recvSlots_V;
            RDY_ARRIVED_BUF <= Rdy_ARRIVED_BUF_VAR;
            RDY_ARRIVED_BUF_CHANGE <= Rdy_ARRIVED_BUF_CHANGE_VAR;
                                    
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
                        
            srdyPhaseCounter <= srdyPhaseCounter + 1;
            if srdyPhaseCounter + 1 = srdyPhaseCounterLimit then
                srdyPhaseCounter <= 0;
            end if;
            
        END IF;

    END PROCESS;
    
END Behavioral;
