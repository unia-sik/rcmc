library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libproc.all;
use work.libnode.all;
USE work.constants.ALL;

entity nocunit is
    generic (
        id : integer;
        count : integer;
        nocdim : std_logic_vector(63 downto 0)
    );
    
    port (
        clk               : in  std_logic;
        rst_n             : in  std_logic;
        LED               : out std_logic;
        NorthOut          : OUT P_PORT_VERTICAL;
        SouthIn           : IN  P_PORT_VERTICAL;
        EastOut           : OUT P_PORT_HORIZONTAL;
        WestIn            : IN  P_PORT_HORIZONTAL;
        
        RdyIn_1           : IN  P_PORT_RDY;
        RdyOut_1          : OUT P_PORT_RDY;
        
        RdyIn_2           : IN  P_PORT_RDY;
        RdyOut_2          : OUT P_PORT_RDY;
        
        CoreAddress       : IN  Address
    );
    
end entity nocunit;

architecture rtl of nocunit is    
    signal BufferToNode : P_PORT_BUFFER;
    signal BufferToNode2 : P_PORT_BUFFER;
    signal RdyToNode : P_PORT_BUFFER;
    signal NodeToBuffer : P_PORT_BUFFER;
    
    
    
    signal BufferToProcessor : P_PORT_BUFFER;
    signal BufferToProcessor2 : P_PORT_BUFFER;
    
    signal ProcessorToBuffer : P_PORT_BUFFER;
    signal ProcessorRequest : REQUEST_PORT;
    signal SendBufferFull : std_logic;
    signal RecvBufferEmpty : std_logic;
    signal NodeClear : std_logic;
    signal RdyClear : std_logic;
    signal RecvClear : std_logic;
    signal LocalRdyIn_1 : RdyAddress;
    signal LocalRdyIn_2 : RdyAddress;
    signal LocalRdyOut :  P_PORT_RDY;
    signal rdaddress : STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
    signal wraddress : STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
    signal rdclocken : std_logic;
    
    signal rdaddress_recv : STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
    signal wraddress_recv : STD_LOGIC_VECTOR(HeapSizeBits - 1 DOWNTO 0);
    signal rdclocken_recv : std_logic;
    
    signal BarrierConfigToNode: BarrierConfig;
    signal BarrierSetToProcessor:  std_logic;
    
    signal cornerBufferOut_1  : P_PORT_HORIZONTAL;
    signal cornerBufferWriteAddr_1 : std_logic_vector(HeapSizeBits - 1 downto 0);  
    signal cornerBufferWriteEnable_1 : std_logic;
    signal cornerBufferReadAddr_1 : std_logic_vector(HeapSizeBits - 1 downto 0);             
    signal cornerBufferIn_1 : P_PORT_HORIZONTAL;
    
    
    signal cornerBufferOut_2  : P_PORT_HORIZONTAL;
    signal cornerBufferWriteAddr_2 : std_logic_vector(HeapSizeBits - 1 downto 0);          
    signal cornerBufferWriteEnable_2 : std_logic;    
    signal cornerBufferReadAddr_2 : std_logic_vector(HeapSizeBits - 1 downto 0);             
    signal cornerBufferIn_2 : P_PORT_HORIZONTAL;
begin


    cpu_inst : cpu_top
    generic map (
        id     => id,
        count  => count,
        nocdim => nocdim
    )
    
    port map (
        clk          => clk,
        rst_n        => rst_n,
        NodeToBuffer => BufferToProcessor,
        NodeToBuffer2 => BufferToProcessor2,
        RecvBufferEmpty => RecvBufferEmpty,
        
        BarrierConfigOut => BarrierConfigToNode,
        BarrierSetIn     => BarrierSetToProcessor,
        
        nbo          => ProcessorToBuffer,
        nbsf         => SendBufferFull,
        LocalRdyIn_1 => LocalRdyIn_1,
        LocalRdyIn_2 => LocalRdyIn_2,
        recvClear    => RecvClear
    );
    
    nis_inst : NetworkInterfaceSend
    port map (
        Clk               => clk,
        Rst               => rst_n,
        ProcessorToBuffer => ProcessorToBuffer,
        BufferToNode      => BufferToNode2,
        isFull            => SendBufferFull,
        rdclocken         => rdclocken,
        LocalRdyOut       => LocalRdyOut,
        NodeClear         => NodeClear,
        RdyClear          => RdyClear,
        
        rdaddress => rdaddress,
        wraddress => wraddress
    );

    nis_flit : flit_buffer
    port map (
        rdaddress => rdaddress,
        rdclock   => clk,
        rdclocken => rdclocken,
        
        data      => ProcessorToBuffer,
        wraddress => wraddress,
        wrclock   => clk,
        wrclocken => '1',
        wren      => ProcessorToBuffer.DataAvailable,
        
        q         => BufferToNode
    );

    nir_inst : NetworkInterfaceRecv
    port map (
        Clk               => clk,
        Rst               => rst_n,
        
        NodeToBuffer => NodeToBuffer,
        BufferToProcessor      => BufferToProcessor2,
        
        isEmpty            => RecvBufferEmpty,
        rdclocken         => rdclocken_recv,
        NodeClear         => recvClear,
        
        rdaddress => rdaddress_recv,
        wraddress => wraddress_recv
    );

    nir_flit : flit_buffer
    port map (
        rdaddress => rdaddress_recv,
        rdclock   => clk,
        rdclocken => rdclocken_recv,
        
        data      => NodeToBuffer,
        wraddress => wraddress_recv,
        wrclock   => clk,
        wrclocken => '1',
        wren      => NodeToBuffer.DataAvailable,
        
        q         => BufferToProcessor
    );
    
    
    corner_buffer_inst_1 : flit_buffer2
    port map (
        rdaddress => cornerBufferReadAddr_1,
        rdclock   => clk,
        rdclocken => '1',
        
        data      => cornerBufferOut_1,
        wraddress => cornerBufferWriteAddr_1,
        wrclock   => clk,
        wrclocken => '1',
        wren      => cornerBufferWriteEnable_1,
        
        q         => cornerBufferIn_1
    );
    
    corner_buffer_inst_2 : flit_buffer2
    port map (
        rdaddress => cornerBufferReadAddr_2,
        rdclock   => clk,
        rdclocken => '1',
        
        data      => cornerBufferOut_2,
        wraddress => cornerBufferWriteAddr_2,
        wrclock   => clk,
        wrclocken => '1',
        wren      => cornerBufferWriteEnable_2,
        
        q         => cornerBufferIn_2    
    );   
    
    node_inst_pnoo: if conf_routing = CONF_PNOO generate
        node_inst : pnoo_node
        port map (           
            Clk            => clk,
            Rst            => rst_n,
            CoreAddress    => CoreAddress,
            
            NodeClear      => NodeClear,
            RdyClear       => RdyClear,
            RecvClear      => RecvClear,
            
            NorthOut       => NorthOut,
            SouthIn        => SouthIn,
            EastOut        => EastOut,
            WestIn         => WestIn,
            
            RdyIn_1        => RdyIn_1,
            RdyIn_2        => RdyIn_2,
            
            RdyOut_1       => RdyOut_1,
            RdyOut_2       => RdyOut_2,
                        
            LocalIn        => BufferToNode,
            LocalOut       => NodeToBuffer,
                        
            LocalRdyIn_1   => LocalRdyIn_1,
            LocalRdyIn_2   => LocalRdyIn_2,
            LocalRdyOut    => LocalRdyOut
        );
    end generate node_inst_pnoo;
    
    node_inst_pnoo_double_corner: if conf_routing = CONF_PNOO_DOUBLE_CORNER_BUFFER generate   
        node_inst : pnoo_node_double_corner
        port map (           
            Clk            => clk,
            Rst            => rst_n,
            CoreAddress    => CoreAddress,
            
            NodeClear      => NodeClear,
            RdyClear       => RdyClear,
            RecvClear      => RecvClear,
            
            NorthOut       => NorthOut,
            SouthIn        => SouthIn,
            EastOut        => EastOut,
            WestIn         => WestIn,
            
            RdyIn_1        => RdyIn_1,
            RdyIn_2        => RdyIn_2,
            
            RdyOut_1       => RdyOut_1,
            RdyOut_2       => RdyOut_2,
                        
            LocalIn_1      => BufferToNode,
            LocalIn_2      => BufferToNode2,
            LocalOut       => NodeToBuffer,
                        
            LocalRdyIn_1   => LocalRdyIn_1,
            LocalRdyIn_2   => LocalRdyIn_2,
            LocalRdyOut    => LocalRdyOut,
            
            BarrierConfigIn => BarrierConfigToNode,
            BarrierSetOut => BarrierSetToProcessor,
            
            cornerBufferOut_1  => cornerBufferOut_1,
            cornerBufferWriteAddr_1 => cornerBufferWriteAddr_1,       
            cornerBufferWriteEnable_1 => cornerBufferWriteEnable_1,
            cornerBufferReadAddr_1 => cornerBufferReadAddr_1,
            cornerBufferIn_1 => cornerBufferIn_1,
            
            cornerBufferOut_2  => cornerBufferOut_2,
            cornerBufferWriteAddr_2 => cornerBufferWriteAddr_2,            
            cornerBufferWriteEnable_2 => cornerBufferWriteEnable_2,
            cornerBufferReadAddr_2 => cornerBufferReadAddr_2,
            cornerBufferIn_2 => cornerBufferIn_2
        );
    end generate node_inst_pnoo_double_corner;
        
    node_inst_pnoo_sort: if conf_routing = CONF_PNOO_SORT generate
        node_inst : pnoo_node_sort
        port map (           
            Clk            => clk,
            Rst            => rst_n,
            CoreAddress    => CoreAddress,
            
            NodeClear      => NodeClear,
            RdyClear       => RdyClear,
            RecvClear      => RecvClear,
            
            NorthOut       => NorthOut,
            SouthIn        => SouthIn,
            EastOut        => EastOut,
            WestIn         => WestIn,
            
            RdyIn_1        => RdyIn_1,
            RdyIn_2        => RdyIn_2,
            
            RdyOut_1       => RdyOut_1,
            RdyOut_2       => RdyOut_2,
                        
            LocalIn        => BufferToNode,
            LocalOut       => NodeToBuffer,
                        
            LocalRdyIn_1   => LocalRdyIn_1,
            LocalRdyIn_2   => LocalRdyIn_2,
            LocalRdyOut    => LocalRdyOut
        );
    end generate node_inst_pnoo_sort;
    
    node_inst_pnoo_empty: if conf_routing = CONF_PNOO_EMPTY generate
        node_inst : pnoo_node_empty
        port map (           
            Clk            => clk,
            Rst            => rst_n
        );
    end generate node_inst_pnoo_empty;
    
    node_inst_pnoo_no_rdy: if conf_routing = CONF_PNOO_NO_RDY generate
        node_inst : pnoo_node_no_rdy
        port map (           
            Clk            => clk,
            Rst            => rst_n,
            CoreAddress    => CoreAddress,
            
            NodeClear      => NodeClear,
            
            NorthOut       => NorthOut,
            SouthIn        => SouthIn,
            EastOut        => EastOut,
            WestIn         => WestIn,
                        
            LocalIn_1      => BufferToNode,
            LocalIn_2      => BufferToNode2,
            LocalOut       => NodeToBuffer,
                                    
            cornerBufferOut_1  => cornerBufferOut_1,
            cornerBufferWriteAddr_1 => cornerBufferWriteAddr_1,       
            cornerBufferWriteEnable_1 => cornerBufferWriteEnable_1,
            cornerBufferReadAddr_1 => cornerBufferReadAddr_1,
            cornerBufferIn_1 => cornerBufferIn_1,
            
            cornerBufferOut_2  => cornerBufferOut_2,
            cornerBufferWriteAddr_2 => cornerBufferWriteAddr_2,            
            cornerBufferWriteEnable_2 => cornerBufferWriteEnable_2,
            cornerBufferReadAddr_2 => cornerBufferReadAddr_2,
            cornerBufferIn_2 => cornerBufferIn_2
        );
    end generate node_inst_pnoo_no_rdy;
    
    node_inst_pnoo_srr: if conf_routing = CONF_PNOO_SRR generate
        node_inst : pnoo_node_srr
        port map (           
            Clk            => clk,
            Rst            => rst_n,
            CoreAddress    => CoreAddress,
            
            NodeClear      => NodeClear,
            RdyClear       => RdyClear,
            
            NorthOut       => NorthOut,
            SouthIn        => SouthIn,
            EastOut        => EastOut,
            WestIn         => WestIn,
            
            RdyIn_1        => RdyIn_1,
            
            RdyOut_1       => RdyOut_1,
                        
            LocalIn_1        => BufferToNode,
            LocalIn_2        => BufferToNode2,
            LocalOut       => NodeToBuffer,
                        
            LocalRdyIn_1   => LocalRdyIn_1,
            LocalRdyOut    => LocalRdyOut,
            
            cornerBufferOut_1  => cornerBufferOut_1,
            cornerBufferWriteAddr_1 => cornerBufferWriteAddr_1,       
            cornerBufferWriteEnable_1 => cornerBufferWriteEnable_1,
            cornerBufferReadAddr_1 => cornerBufferReadAddr_1,
            cornerBufferIn_1 => cornerBufferIn_1,
            
            cornerBufferOut_2  => cornerBufferOut_2,
            cornerBufferWriteAddr_2 => cornerBufferWriteAddr_2,            
            cornerBufferWriteEnable_2 => cornerBufferWriteEnable_2,
            cornerBufferReadAddr_2 => cornerBufferReadAddr_2,
            cornerBufferIn_2 => cornerBufferIn_2
        );
    end generate node_inst_pnoo_srr;
    
    node_inst_pnoo_drr: if conf_routing = CONF_PNOO_DRR generate
        node_inst : pnoo_node_drr
        port map (           
            Clk            => clk,
            Rst            => rst_n,
            CoreAddress    => CoreAddress,
            
            NodeClear      => NodeClear,
            RdyClear       => RdyClear,
            
            NorthOut       => NorthOut,
            SouthIn        => SouthIn,
            EastOut        => EastOut,
            WestIn         => WestIn,
            
            RdyIn_1        => RdyIn_1,
            RdyIn_2        => RdyIn_2,
            
            RdyOut_1       => RdyOut_1,
            RdyOut_2       => RdyOut_2,
                        
            LocalIn_1        => BufferToNode,
            LocalIn_2        => BufferToNode2,
            LocalOut       => NodeToBuffer,
                        
            LocalRdyIn_1   => LocalRdyIn_1,
            LocalRdyIn_2   => LocalRdyIn_2,
            LocalRdyOut    => LocalRdyOut,
            
            cornerBufferOut_1  => cornerBufferOut_1,
            cornerBufferWriteAddr_1 => cornerBufferWriteAddr_1,       
            cornerBufferWriteEnable_1 => cornerBufferWriteEnable_1,
            cornerBufferReadAddr_1 => cornerBufferReadAddr_1,
            cornerBufferIn_1 => cornerBufferIn_1,
            
            cornerBufferOut_2  => cornerBufferOut_2,
            cornerBufferWriteAddr_2 => cornerBufferWriteAddr_2,            
            cornerBufferWriteEnable_2 => cornerBufferWriteEnable_2,
            cornerBufferReadAddr_2 => cornerBufferReadAddr_2,
            cornerBufferIn_2 => cornerBufferIn_2
        );
    end generate node_inst_pnoo_drr;
    
    node_inst_pnoo_cg: if conf_routing = CONF_PNOO_CG generate
        node_inst : pnoo_node_cg
        port map (           
            Clk            => clk,
            Rst            => rst_n,
            CoreAddress    => CoreAddress,
            
            NodeClear      => NodeClear,
            RdyClear       => RdyClear,
            
            NorthOut       => NorthOut,
            SouthIn        => SouthIn,
            EastOut        => EastOut,
            WestIn         => WestIn,
            
            RdyIn_1        => RdyIn_1,
            RdyIn_2        => RdyIn_2,
            
            RdyOut_1       => RdyOut_1,
            RdyOut_2       => RdyOut_2,
                        
            LocalIn_1        => BufferToNode,
            LocalIn_2        => BufferToNode2,
            LocalOut       => NodeToBuffer,
                        
            LocalRdyIn_1   => LocalRdyIn_1,
            LocalRdyIn_2   => LocalRdyIn_2,
            LocalRdyOut    => LocalRdyOut,
            
            cornerBufferOut_1  => cornerBufferOut_1,
            cornerBufferWriteAddr_1 => cornerBufferWriteAddr_1,       
            cornerBufferWriteEnable_1 => cornerBufferWriteEnable_1,
            cornerBufferReadAddr_1 => cornerBufferReadAddr_1,
            cornerBufferIn_1 => cornerBufferIn_1,
            
            cornerBufferOut_2  => cornerBufferOut_2,
            cornerBufferWriteAddr_2 => cornerBufferWriteAddr_2,            
            cornerBufferWriteEnable_2 => cornerBufferWriteEnable_2,
            cornerBufferReadAddr_2 => cornerBufferReadAddr_2,
            cornerBufferIn_2 => cornerBufferIn_2
        );
    end generate node_inst_pnoo_cg;
    
    
    LED <= SendBufferFull;
    
end architecture;
