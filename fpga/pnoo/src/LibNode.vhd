LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE work.constants.ALL;


PACKAGE LibNode IS   

    -- for Routing Process
    type Cycle is
    record
        X: Natural;
        Y: Natural;
    end record;

    type Address is
    record            
        X: std_logic_vector(Address_Length_X-1 DOWNTO 0);
        Y: std_logic_vector(Address_Length_Y-1 DOWNTO 0);
    end record;
    
    type RdyAddress is
    record            
        X: std_logic_vector(Address_Length_X-1 DOWNTO 0);
        Y: std_logic_vector(Address_Length_Y-1 DOWNTO 0);
        Toogle: std_logic;
    end record;
    
    type AddressPort is
    record            
        AddressReceiver:    Address;
        AddressSender:        Address;    
    end record;

    
    type BarrierConfig is
    record
        StartAddr: Address;
        EndAddr: Address;
        Set:   std_logic;
    end record;
    
    type P_PORT_RDY is
    record            
--         Data:                 std_logic_vector(Data_Length-1 DOWNTO 0);
        --Address:             std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
        Address:                AddressPort;    
--         DataAvailable:     std_logic;
--         IsRDYInstr:        std_logic;
        IsValid:              std_logic;
        IsFirst:              std_logic;
        IsBarrier:            std_logic;
    end record;
    
    type P_PORT_VERTICAL is
    record            
        Data:                 std_logic_vector(Data_Length-1 DOWNTO 0);
        --Address:             std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
        Address:                AddressPort;    
        DataAvailable:     std_logic;
        IsRdyInstr:        std_logic;
    end record;
    
    type P_PORT_HORIZONTAL is
    record            
        Data:                 std_logic_vector(Data_Length-1 DOWNTO 0);
        --Address:             std_logic_vector((Address_Length_X + Address_Length_Y)*2-1  DOWNTO 0);
        Address:                AddressPort;    
        DataAvailable:     std_logic;        
        IsRDYInstr:        std_logic;
    end record;
    
    TYPE REQUEST_PORT is
    RECORD
        Address:            std_logic_vector((Address_Length_X + Address_Length_Y)-1  DOWNTO 0);
        Request:            std_logic;
        SpecificRequest:    std_logic;              
        ProbeRequest:        std_logic; --als peek, daten rückliefern aber nicht entfernen
    END RECORD;

    type P_PORT_BUFFER is
    record            
        Data:                 std_logic_vector(Data_Length-1 DOWNTO 0);
         --Address:             std_logic_vector((Address_Length_X + Address_Length_Y)-1  DOWNTO 0);
        Address:        Address;
        DataAvailable:        std_logic;    
        IsRDYInstr:        std_logic;  
    end record;
    
    type FIFOEntry is
    record            
        Data:                 std_logic_vector(Data_Length-1 DOWNTO 0);
        Address:                AddressPort;
        IsRDYInstr:        std_logic;    
    end record;
    
    TYPE BusyCounterReceive_T IS ARRAY (0 TO Dimension - 1) OF natural;
    
    component pnoo_node
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
            
            RdyIn_1        : in P_PORT_RDY;
            RdyIn_2        : in P_PORT_RDY;
            
            RdyOut_1       : out P_PORT_RDY;           
            RdyOut_2       : out P_PORT_RDY;
                        
            LocalIn         : in  P_PORT_BUFFER;  
            LocalOut        : out P_PORT_BUFFER;
                        
            LocalRdyIn_1   : out RdyAddress;
            LocalRdyIn_2   : out RdyAddress;
            LocalRdyOut    : in  P_PORT_RDY
        );
    END component;
    
    component pnoo_node_double_corner
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
            
            RdyIn_1        : in P_PORT_RDY;
            RdyIn_2        : in P_PORT_RDY;
            
            RdyOut_1       : out P_PORT_RDY;           
            RdyOut_2       : out P_PORT_RDY;
                        
            LocalIn_1       : in  P_PORT_BUFFER;  
            LocalIn_2       : in  P_PORT_BUFFER;  
            LocalOut        : out P_PORT_BUFFER;
                        
            LocalRdyIn_1   : out RdyAddress;
            LocalRdyIn_2   : out RdyAddress;
            LocalRdyOut    : in  P_PORT_RDY;
            
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
    END component;
    
        
    component pnoo_node_sort
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
            
            RdyIn_1        : in P_PORT_RDY;
            RdyIn_2        : in P_PORT_RDY;
            
            RdyOut_1       : out P_PORT_RDY;           
            RdyOut_2       : out P_PORT_RDY;
                        
            LocalIn         : in  P_PORT_BUFFER;  
            LocalOut        : out P_PORT_BUFFER;
                        
            LocalRdyIn_1   : out RdyAddress;
            LocalRdyIn_2   : out RdyAddress;
            LocalRdyOut    : in  P_PORT_RDY
        );
    END component;
    
    component pnoo_node_empty
    PORT (        
            Clk             : in std_logic;
            Rst             : in std_logic
        );
    END component;
    
    component pnoo_node_no_rdy
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
    END component;
    
    component pnoo_node_srr
    PORT (        
            Clk             : in std_logic;
            Rst             : in std_logic;            
            CoreAddress     : in  Address;
            
            NodeClear       : out std_logic;   
            RdyClear        : out std_logic;
            
            NorthOut        : out P_PORT_VERTICAL;
            SouthIn         : in  P_PORT_VERTICAL;
            EastOut         : out P_PORT_HORIZONTAL;
            WestIn          : in  P_PORT_HORIZONTAL;
            
            RdyIn_1        : in P_PORT_RDY;
            
            RdyOut_1       : out P_PORT_RDY;    
                        
            LocalIn_1         : in  P_PORT_BUFFER;  
            LocalIn_2         : in  P_PORT_BUFFER; 
            LocalOut        : out P_PORT_BUFFER;
                        
            LocalRdyIn_1   : out RdyAddress;
            LocalRdyOut    : in  P_PORT_RDY;
            
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
    END component;
    
    component pnoo_node_drr
    PORT (        
            Clk             : in std_logic;
            Rst             : in std_logic;            
            CoreAddress     : in  Address;
            
            NodeClear       : out std_logic;   
            RdyClear        : out std_logic;
            
            NorthOut        : out P_PORT_VERTICAL;
            SouthIn         : in  P_PORT_VERTICAL;
            EastOut         : out P_PORT_HORIZONTAL;
            WestIn          : in  P_PORT_HORIZONTAL;
            
            RdyIn_1        : in P_PORT_RDY;
            RdyIn_2        : in P_PORT_RDY;
            
            RdyOut_1       : out P_PORT_RDY;           
            RdyOut_2       : out P_PORT_RDY;
                        
            LocalIn_1         : in  P_PORT_BUFFER;  
            LocalIn_2         : in  P_PORT_BUFFER; 
            LocalOut        : out P_PORT_BUFFER;
                        
            LocalRdyIn_1   : out RdyAddress;
            LocalRdyIn_2   : out RdyAddress;
            LocalRdyOut    : in  P_PORT_RDY;
            
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
    END component;
    
    component pnoo_node_cg
    PORT (        
            Clk             : in std_logic;
            Rst             : in std_logic;            
            CoreAddress     : in  Address;
            
            NodeClear       : out std_logic;   
            RdyClear        : out std_logic;
            
            NorthOut        : out P_PORT_VERTICAL;
            SouthIn         : in  P_PORT_VERTICAL;
            EastOut         : out P_PORT_HORIZONTAL;
            WestIn          : in  P_PORT_HORIZONTAL;
            
            RdyIn_1        : in P_PORT_RDY;
            RdyIn_2        : in P_PORT_RDY;
            
            RdyOut_1       : out P_PORT_RDY;           
            RdyOut_2       : out P_PORT_RDY;
                        
            LocalIn_1         : in  P_PORT_BUFFER;  
            LocalIn_2         : in  P_PORT_BUFFER;   
            LocalOut        : out P_PORT_BUFFER;
                        
            LocalRdyIn_1   : out RdyAddress;
            LocalRdyIn_2   : out RdyAddress;
            LocalRdyOut    : in  P_PORT_RDY;
            
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
    END component;
    
    COMPONENT NetworkInterfaceSend 
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
    END COMPONENT;
    
    COMPONENT NetworkInterfaceRecv IS
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
    END COMPONENT;
    
    COMPONENT flit_buffer IS 
    PORT(        
        rdaddress : IN  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        rdclock   : IN  STD_LOGIC;
        rdclocken : IN  STD_LOGIC := '1';
        
        data      : IN  P_PORT_BUFFER;
        wraddress : IN  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        wrclock   : IN  STD_LOGIC := '1';
        wrclocken : IN  STD_LOGIC := '1';
        wren      : IN  STD_LOGIC := '0';
        
        q         : OUT P_PORT_BUFFER
    );
    END COMPONENT;
    
    COMPONENT flit_buffer2 IS 
    PORT(        
        rdaddress : IN  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        rdclock   : IN  STD_LOGIC;
        rdclocken : IN  STD_LOGIC := '1';
        
        data      : IN  P_PORT_HORIZONTAL;
        wraddress : IN  STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
        wrclock   : IN  STD_LOGIC := '1';
        wrclocken : IN  STD_LOGIC := '1';
        wren      : IN  STD_LOGIC := '0';
        
        q         : OUT P_PORT_HORIZONTAL
    );
    END COMPONENT;

END LibNode;
