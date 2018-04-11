library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;

entity mcodefile is
    port(
        clk          : in  std_logic;
        rst_n        : in  std_logic;
        mcoderi      : in  mcodefile_read_in_type;
        mcodewi      : in  mcodefile_write_in_type;
        mcodeo       : out mcodefile_out_type;
        global_stall : in  std_logic
    );

end mcodefile;

architecture rtl of mcodefile is
	type MRegisters is array (0 to 15) of std_logic_vector (63 downto 0);
	
  signal mcode_register : MRegisters := (
    0 => x"000000004034B4B5", -- binary32
    1 => x"4006969696966D32", -- binary64: 48/17 = 2.82
    2 => x"000000003FF0F0F1", -- binary32
    3 => x"3FFE1E1E1E1E5C35", -- binary64: 32/17 = 1.88
    4 => x"000000003F800000", -- binary32
    5 => x"3FF0000000000000", -- binary64: 1.0
    6 => x"000000003FC00000",
    7 => x"3FF8000000000000", -- binary64: 1.5
    8 => x"000000003F000000",
    9 => x"3FE0000000000000", -- binary64: 0.5
    others => (others => '0')
    );
    
    signal read_address1 : std_logic_vector(3 downto 0);
    signal read_address2 : std_logic_vector(3 downto 0);
    signal read_address3 : std_logic_vector(3 downto 0);
    
    
    signal write_addr1 : std_logic_vector(3 downto 0);
    signal write_data1 : std_logic_vector(63 downto 0);
    signal write_en1 : std_logic;
    
    signal write_addr2 : std_logic_vector(3 downto 0);
    signal write_data2 : std_logic_vector(63 downto 0);
    signal write_en2 : std_logic;
    
begin
	process(clk, rst_n)
		
	begin
		
        if rst_n = '0' then
            read_address1       <= (others => '0');
            read_address2       <= (others => '0');
            read_address3       <= (others => '0');
        elsif (rising_edge(clk)) then
            if global_stall = '0' then
                if mcoderi.rclke = '1' then
                    read_address1       <= mcoderi.raddr1;
                    read_address2       <= mcoderi.raddr2;
                    read_address3       <= mcoderi.raddr3;
                end if;
                
                write_addr1 <= mcodewi.waddr1;
                write_data1 <= mcodewi.wdata1;
                write_en1 <= mcodewi.we1;
                
                write_addr2 <= mcodewi.waddr2;
                write_data2 <= mcodewi.wdata2;
                write_en2 <= mcodewi.we2;
                
                
                -- write
                if mcodewi.we1 = '1' then
                    mcode_register(to_integer(unsigned(mcodewi.waddr1))) <= mcodewi.wdata1;
                end if;
                
                -- write
                if mcodewi.we2 = '1' then
                    mcode_register(to_integer(unsigned(mcodewi.waddr2))) <= mcodewi.wdata2;
                end if;
                
            end if;
        end if;
        
    end process;

    	
    process(read_address1,read_address2,read_address3,mcode_register)
    	
    begin
    	
        mcodeo.rdata1 <= mcode_register(to_integer(unsigned(read_address1)));
        mcodeo.rdata2 <= mcode_register(to_integer(unsigned(read_address2)));
        mcodeo.rdata3 <= mcode_register(to_integer(unsigned(read_address3)));
        
    end process;
end rtl;
