library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;
use work.libproc.all;

entity mcodecache is
    port(
        clk          : in  std_logic;
        mcci         : in  mcodecache_in_type;
        mcco         : out mcodecache_out_type;
		clken        : in  std_logic
    );

end mcodecache;

architecture rtl of mcodecache is
    
    
--    type MCodeDivisionSingle is array (0 to 7) of std_logic_vector (31 downto 0);
--    signal mcode_division_single : MCodeDivisionSingle := (0 => x"0025864B" ,1 => x"20C586CB" ,2 => x"60C68643" ,3 => x"20C586CB" ,
--    														4 => x"60C68643" ,5 => x"20C586CB" ,6 => x"60C68643" ,7 => x"10C507D3");
--    
--    type MCodeDivisionDouble is array (0 to 9) of std_logic_vector (31 downto 0);
--    signal mcode_division_double : MCodeDivisionDouble := (0 => x"0A35864B" ,1 => x"2AC586CB" ,2 => x"62C68643" ,3 => x"2AC586CB" ,
--    														4 => x"62C68643" ,5 => x"2AC586CB" ,6 => x"62C68643" ,7 => x"2AC586CB" ,
--    														8 => x"62C68643" ,9 => x"12C507D3");
--    
--    type MCodeSquareRootSingle is array (0 to 10) of std_logic_vector (31 downto 0);
--    signal mcode_squareroot_single : MCodeSquareRootSingle := (0 => x"108505D3" ,1 => x"10C606D3" ,2 => x"30D586CB" ,3 => x"10D60653" ,
--    															4 => x"10C606D3" ,5 => x"30D586CB" ,6 => x"10D60653" ,7 => x"10C606D3" ,
--    															8 => x"30D586CB" ,9 => x"10D60653" ,10 => x"10A607D3");
--    
--    type MCodeSquareRootDouble is array (0 to 13) of std_logic_vector (31 downto 0);
--    signal mcode_squareroot_double : MCodeSquareRootDouble := (0 => x"129505D3" ,1 => x"12C606D3" ,2 => x"3AD586CB" ,3 => x"12D60653" ,
--    															4 => x"12C606D3" ,5 => x"3AD586CB" ,6 => x"12D60653" ,7 => x"12C606D3" ,
--    															8 => x"3AD586CB" ,9 => x"12D60653" ,10 => x"12C606D3" ,11 => x"3AD586CB" ,
--    															12 => x"12D60653" ,13 => x"12A607D3");
    
    
    type MCodeInstructions is array (0 to 3, 0 to 31) of std_logic_vector (31 downto 0);
    
--    signal mcode_inst : MCodeInstructions := (	0 => (0 => x"0025864B" ,1 => x"20C586CB" ,2 => x"60C68643" ,3 => x"20C586CB" ,
--    												4 => x"60C68643" ,5 => x"20C586CB" ,6 => x"60C68643" ,7 => x"10C507D3" ,
--    												others => x"00000013"),
--    											1 => (0 => x"0A35864B" ,1 => x"2AC586CB" ,2 => x"62C68643" ,3 => x"2AC586CB" ,
--    												4 => x"62C68643" ,5 => x"2AC586CB" ,6 => x"62C68643" ,7 => x"2AC586CB" ,
--    												8 => x"62C68643" ,9 => x"12C507D3" ,others => x"00000013"),
--    											2 => (0 => x"108505D3" ,1 => x"10C606D3" ,2 => x"30D586CB" ,3 => x"10D60653" ,
--    												4 => x"10C606D3" ,5 => x"30D586CB" ,6 => x"10D60653" ,7 => x"10C606D3" ,
--    												8 => x"30D586CB" ,9 => x"10D60653" ,10 => x"10C606D3" ,11 => x"30D586CB" ,
--    												12 => x"10D60653",13 => x"10A607D3" ,others => x"00000013"),	
--    											3 => (0 => x"129505D3" ,1 => x"12C606D3" ,2 => x"3AD586CB" ,3 => x"12D60653" ,
--    												4 => x"12C606D3" ,5 => x"3AD586CB" ,6 => x"12D60653" ,7 => x"12C606D3" ,
--    												8 => x"3AD586CB" ,9 => x"12D60653" ,10 => x"12C606D3" ,11 => x"3AD586CB" ,
--    												12 => x"12D60653" ,13 => x"12C606D3" ,14 => x"3AD586CB" ,15 => x"12D60653",
--    												16 => x"12A607D3" ,others => x"00000013"));
    
    
    
--    signal mcode_inst : MCodeInstructions := (	0 => (0 => x"0025864B" ,1 => x"20C586CB" ,2 => x"60C68643" ,3 => x"20C586CB" ,
--    												4 => x"60C68643" ,5 => x"20C586CB" ,6 => x"60C68643" ,7 => x"10C507D3" ,
--    												others => x"00000013"),
--    											1 => (0 => x"0A35864B" ,1 => x"2AC586CB" ,2 => x"62C68643" ,3 => x"2AC586CB" ,
--    												4 => x"62C68643" ,5 => x"2AC586CB" ,6 => x"62C68643" ,7 => x"2AC586CB" ,
--    												8 => x"62C68643" ,9 => x"12C507D3" ,others => x"00000013"),
--    											2 => (0 => x"108505D3" ,1 => x"10C606D3" ,2 => x"30D586CB" ,3 => x"10D60653" ,
--    												4 => x"10C606D3" ,5 => x"30D586CB" ,6 => x"10D60653" ,7 => x"10C606D3" ,
--    												8 => x"30D586CB" ,9 => x"10D60653" ,10 => x"10A607D3" ,others => x"00000013"),	
--    											3 => (0 => x"129505D3" ,1 => x"12C606D3" ,2 => x"3AD586CB" ,3 => x"12D60653" ,
--    												4 => x"12C606D3" ,5 => x"3AD586CB" ,6 => x"12D60653" ,7 => x"12C606D3" ,
--    												8 => x"3AD586CB" ,9 => x"12D60653" ,10 => x"12C606D3" ,11 => x"3AD586CB" ,
--    												12 => x"12D60653" ,13 => x"12A607D3" ,others => x"00000013"));
    												
    
    signal mcode_inst : MCodeInstructions := (	0 => (0 => x"0025864B" ,1 => x"20C586CB" ,2 => x"60C68643" ,3 => x"20C586CB" ,
    												4 => x"60C68643" ,5 => x"20C586CB" ,6 => x"60C68643" ,7 => x"10C507D3" ,
    												others => x"00000013"),
    											1 => (0 => x"0A35864B" ,1 => x"2AC586CB" ,2 => x"62C68643" ,3 => x"2AC586CB" ,
    												4 => x"62C68643" ,5 => x"2AC586CB" ,6 => x"62C68643" ,7 => x"2AC586CB" ,
    												8 => x"62C68643" ,9 => x"12C507D3" ,others => x"00000013"),
    											2 => (0 => x"10C505D3" ,1 => x"108606D3" ,2 => x"40D5874B" ,3 => x"58E585C3" ,
    												4 => x"68E686C3" ,5 => x"50B587CB" ,6 => x"58F685C3" ,7 => x"50B587CB" ,
    												8 => x"58F687C3" ,others => x"00000013"),
    											3 => (0 => x"12C505D3" ,1 => x"129606D3" ,2 => x"4AD5874B" ,3 => x"5AE585C3" ,
    												4 => x"6AE686C3" ,5 => x"4AD5874B" ,6 => x"5AE585C3" ,7 => x"6AE686C3" ,
    												8 => x"52B587CB" ,9 => x"5AF685C3" ,10 => x"52B587CB" ,11 => x"5AF687C3" ,
    												others => x"00000013"));
    											

begin
		
	process(clk)
		
	begin
		
		if clken = '1' then
			mcco.data(0) <= mcode_inst(0,to_integer(unsigned(mcci.addr(6 downto 2))));
			mcco.data(1) <= mcode_inst(1,to_integer(unsigned(mcci.addr(6 downto 2))));
			mcco.data(2) <= mcode_inst(2,to_integer(unsigned(mcci.addr(6 downto 2))));
			mcco.data(3) <= mcode_inst(3,to_integer(unsigned(mcci.addr(6 downto 2))));
		end if;
		
	end process;
	
end rtl;
