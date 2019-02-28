LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE work.constants.ALL;
USE work.LibNode.ALL;
USE ieee.math_real.floor;

ENTITY pnoo_node IS
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
                        
            LocalIn         : in  P_PORT_BUFFER;  
            LocalOut        : out P_PORT_BUFFER;
                        
            LocalRdyIn_1    : out RdyAddress;
            LocalRdyIn_2    : out RdyAddress;
            LocalRdyOut     : in  P_PORT_RDY
        );
END pnoo_node;


ARCHITECTURE Behavioral OF pnoo_node IS
    signal clkCounter : integer;
    signal phaseCounter : integer;
    signal srdyPhaseCounter : integer;
    signal recvSlots : integer;

    TYPE T_DATA_ARRAY IS ARRAY (0 TO Dimension - 1) OF P_PORT_HORIZONTAL;
    SIGNAL cornerBuffer : T_DATA_ARRAY;

    TYPE T_DATA_ARRAY_ADDR IS ARRAY (0 TO Dimension - 1) OF std_logic_vector(Address_Length_X - 1 downto 0);
    SIGNAL cornerBufferAddr : T_DATA_ARRAY_ADDR;
    
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

    PROCESS (Clk, Rst)
        variable tmpInt : integer;
        variable recvSlots_V : integer;
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
            recvSlots <= 0;
            NodeClear <= '0';   
            RdyClear <= '0';              
            
            for i in 0 to Dimension - 1
            loop
                cornerBuffer(i).DataAvailable <= '0';
                cornerBuffer(i).Address.AddressSender.X <= (others => '0');
                cornerBuffer(i).Address.AddressSender.Y <= (others => '0');
                cornerBuffer(i).Address.AddressReceiver.X <= (others => '0');
                cornerBuffer(i).Address.AddressReceiver.Y <= (others => '0');
                cornerBuffer(i).Data <= (others => '0');
                
                cornerBufferAddr(i) <= (others => '0');
            end loop;
            
            RdyOut_1.Address.AddressReceiver.X <= (others => '0');
            RdyOut_1.Address.AddressReceiver.Y <= (others => '0');
            RdyOut_1.Address.AddressSender.X <= (others => '0');
            RdyOut_1.Address.AddressSender.Y <= (others => '0');
            RdyOut_1.IsValid  <= '0';
            RdyOut_1.IsFirst <= '0';
            
            RdyOut_2.Address.AddressReceiver.X <= (others => '0');
            RdyOut_2.Address.AddressReceiver.Y <= (others => '0');
            RdyOut_2.Address.AddressSender.X <= (others => '0');
            RdyOut_2.Address.AddressSender.Y <= (others => '0');
            RdyOut_2.IsValid  <= '0';
            RdyOut_2.IsFirst <= '0';

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
            
            
            RdyOut_2.Address.AddressReceiver.X <= (others => '0');
            RdyOut_2.Address.AddressReceiver.Y <= (others => '0');
            RdyOut_2.Address.AddressSender.X <= (others => '0');
            RdyOut_2.Address.AddressSender.Y <= (others => '0');
            RdyOut_2.IsValid  <= '0';
            RdyOut_2.IsFirst <= '0';

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
                        
            recvSlots_V := recvSlots;
            RDY_ARRIVED_BUF_VAR := Rdy_ARRIVED_BUF;
            RDY_ARRIVED_BUF_CHANGE_VAR := Rdy_ARRIVED_BUF_CHANGE;
            
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
            --# Sort cornerBuffer by the distance to the destination
            --#######################################################################################################                                  
                       
            if phaseCounter = Dimension - 1 then            
--                 for i in 0 to Dimension - 1
--                 loop
--                     cornerBuffer(i).DataAvailable             <= '0';
--                     cornerBuffer(i).Address.AddressSender.X   <= (others => '0');
--                     cornerBuffer(i).Address.AddressSender.Y   <= (others => '0');
--                     cornerBuffer(i).Address.AddressReceiver.X <= (others => '0');
--                     cornerBuffer(i).Address.AddressReceiver.Y <= (others => '0');
--                     cornerBuffer(i).Data                      <= (others => '0');
--                 end loop;
--             
--             
--                 for i in 1 to Dimension - 1
--                 loop
--                    if cornerBuffer(i).DataAvailable = '1' then
--                         cornerBuffer(calc_distance(CoreAddress.Y, cornerBuffer(i).Address.AddressReceiver.Y)) <= cornerBuffer(i);
--                     end if;
--                 end loop;
            end if;  
            
            --#######################################################################################################
            --# Send next flit to North or to local Node
            --#######################################################################################################
            tmpInt := Dimension - 1 - phaseCounter;
            for i in 0 to Dimension - 1
            loop
                if unsigned(cornerBufferAddr(i)) = tmpInt and cornerBuffer(i).DataAvailable = '1' then
                    if cornerBuffer(i).Address.AddressReceiver.Y /= CoreAddress.Y then
                        NorthOut.Data          <= cornerBuffer(i).Data;
                        NorthOut.Address       <= cornerBuffer(i).Address;
                        NorthOut.DataAvailable <= cornerBuffer(i).DataAvailable;
                        NorthOut.IsRdyInstr   <= cornerBuffer(i).IsRdyInstr;
                        
                        cornerBuffer(i).DataAvailable             <= '0';
--                         cornerBuffer(i).Address.AddressReceiver.X <= (others => '0');
--                         cornerBuffer(i).Address.AddressReceiver.Y <= (others => '0');
                    else
                        LocalOut.Data          <= cornerBuffer(i).Data;    
                        LocalOut.Address       <= cornerBuffer(i).Address.AddressSender;                   
                        LocalOut.DataAvailable <= '1';
                        LocalOut.IsRdyInstr   <= '0';
                        
                        recvSlots_V := recvSlots_V + 1;
                    
                        cornerBuffer(i).DataAvailable             <= '0';
--                         cornerBuffer(i).Address.AddressSender.X   <= (others => '0');
--                         cornerBuffer(i).Address.AddressSender.Y   <= (others => '0');
--                         cornerBuffer(i).Address.AddressReceiver.X <= (others => '0');
--                         cornerBuffer(i).Address.AddressReceiver.Y <= (others => '0');
--                         cornerBuffer(i).Data                      <= (others => '0');
                    end if;
                end if;
            end loop;
            
--             tmpInt := Dimension - 1 - phaseCounter;
--             if tmpInt = calc_distance(CoreAddress.Y, cornerBuffer(tmpInt).Address.AddressReceiver.Y) then            
--                 if cornerBuffer(tmpInt).DataAvailable = '1' and cornerBuffer(tmpInt).Address.AddressReceiver.Y /= CoreAddress.Y then
--                     NorthOut.Data          <= cornerBuffer(tmpInt).Data;
--                     NorthOut.Address       <= cornerBuffer(tmpInt).Address;
--                     NorthOut.DataAvailable <= cornerBuffer(tmpInt).DataAvailable;
--                     NorthOut.IsRdyInstr   <= cornerBuffer(tmpInt).IsRdyInstr;
--                     
--                     if phaseCounter /= Dimension - 1 then  
--                         cornerBuffer(tmpInt).DataAvailable             <= '0';
--                         cornerBuffer(tmpInt).Address.AddressReceiver.X <= CoreAddress.X;
--                         cornerBuffer(tmpInt).Address.AddressReceiver.Y <= CoreAddress.Y;
--                     end if;
--                 elsif cornerBuffer(tmpInt).DataAvailable = '1' and cornerBuffer(tmpInt).Address.AddressReceiver.Y = CoreAddress.Y then
--                     LocalOut.Data          <= cornerBuffer(tmpInt).Data;    
--                     LocalOut.Address       <= cornerBuffer(tmpInt).Address.AddressSender;                   
--                     LocalOut.DataAvailable <= '1';
--                     LocalOut.IsRdyInstr   <= '0';
--                     
--                     recvSlots_V := recvSlots_V + 1;
--                     
--                     if phaseCounter /= Dimension - 1 then  
--                         cornerBuffer(tmpInt).DataAvailable             <= '0';
--                         cornerBuffer(tmpInt).Address.AddressSender.X   <= (others => '0');
--                         cornerBuffer(tmpInt).Address.AddressSender.Y   <= (others => '0');
--                         cornerBuffer(tmpInt).Address.AddressReceiver.X <= (others => '0');
--                         cornerBuffer(tmpInt).Address.AddressReceiver.Y <= (others => '0');
--                         cornerBuffer(tmpInt).Data                      <= (others => '0');
--                     end if;
--                 end if;
--             end if;
            
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
                
                if phaseCounter = Dimension - 1 then
                    cornerBuffer(calc_distance(CoreAddress.Y, WestIn.Address.AddressReceiver.Y)) <= WestIn;
                    cornerBufferAddr(calc_distance(CoreAddress.Y, WestIn.Address.AddressReceiver.Y)) <= std_logic_vector(to_unsigned(calc_distance(CoreAddress.Y, WestIn.Address.AddressReceiver.Y), Address_Length_X));
                else
                    cornerBuffer(Dimension - 1 - phaseCounter) <= WestIn;
                    cornerBufferAddr(Dimension - 1 - phaseCounter) <= std_logic_vector(to_unsigned(Dimension - 1 - phaseCounter, Address_Length_X));
                end if;
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
            
             if srdyPhaseCounter = 0 and LocalRdyOut.IsValid = '1' then
                if recvSlots_V < (HeapSize - Dimension - 1) then
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
            end if; 
            
            --#######################################################################################################
            --# Send data flit to East or insert into cornerBuffer
            --#######################################################################################################
                                    
            if phaseCounter = 0 and
               LocalIn.DataAvailable = '1' and
               LocalIn.IsRdyInstr = '0' and
               (
                    RDY_ARRIVED_BUF_VAR(to_integer(unsigned(LocalIn.Address.Y)))(to_integer(unsigned(LocalIn.Address.X))) = '1' xor
                    Rdy_ARRIVED_BUF_CHANGE_VAR(to_integer(unsigned(LocalIn.Address.Y)))(to_integer(unsigned(LocalIn.Address.X))) = '1'
               ) 
            then                
                if CoreAddress.X = LocalIn.Address.X then
                    cornerBuffer(Dimension - 1).Data <= LocalIn.Data;
                    cornerBuffer(Dimension - 1).Address.AddressReceiver <= LocalIn.Address;
                    cornerBuffer(Dimension - 1).Address.AddressSender.X <= CoreAddress.X;
                    cornerBuffer(Dimension - 1).Address.AddressSender.Y <= CoreAddress.Y;
                    cornerBuffer(Dimension - 1).DataAvailable <= '1';
                    cornerBuffer(Dimension - 1).IsRdyInstr <= '0';
                    
                    cornerBufferAddr(Dimension - 1) <= std_logic_vector(to_unsigned(Dimension - 1, Address_Length_X));
                else
                    EastOut.Data <= LocalIn.Data;
                    EastOut.Address.AddressReceiver <= LocalIn.Address;
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
              
            recvSlots <= recvSlots_V;
            RDY_ARRIVED_BUF <= Rdy_ARRIVED_BUF_VAR;
            RDY_ARRIVED_BUF_CHANGE <= Rdy_ARRIVED_BUF_CHANGE_VAR;
                                    
            clkCounter <= clkCounter + 1;
            phaseCounter <= phaseCounter + 1;
            
            if phaseCounter + 1 = Dimension then
                phaseCounter <= 0;
            end if;
            
            srdyPhaseCounter <= srdyPhaseCounter + 1;
            if srdyPhaseCounter + 1 = srdyPhaseCounterLimit then
                srdyPhaseCounter <= 0;
            end if;
            
        END IF;

    END PROCESS;
    
END Behavioral;
