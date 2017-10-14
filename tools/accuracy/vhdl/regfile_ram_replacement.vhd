LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.numeric_std.all;

ENTITY regfile_ram IS 
	PORT(
		data      : IN  STD_LOGIC_VECTOR(63 DOWNTO 0);
		rdaddress : IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
		rdclock   : IN  STD_LOGIC;
		rdclocken : IN  STD_LOGIC := '1';
		wraddress : IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
		wrclock   : IN  STD_LOGIC := '1';
		wrclocken : IN  STD_LOGIC := '1';
		wren      : IN  STD_LOGIC := '0';
		q         : OUT STD_LOGIC_VECTOR(63 DOWNTO 0);
		reg0	: out std_logic_vector(63 downto 0);
		reg1	: out std_logic_vector(63 downto 0);
		reg2	: out std_logic_vector(63 downto 0);
		reg3	: out std_logic_vector(63 downto 0);
		reg4	: out std_logic_vector(63 downto 0);
		reg5	: out std_logic_vector(63 downto 0);
		reg6	: out std_logic_vector(63 downto 0);
		reg7	: out std_logic_vector(63 downto 0);
		reg8	: out std_logic_vector(63 downto 0);
		reg9	: out std_logic_vector(63 downto 0);
		reg10	: out std_logic_vector(63 downto 0);
		reg11	: out std_logic_vector(63 downto 0);
		reg12	: out std_logic_vector(63 downto 0);
		reg13	: out std_logic_vector(63 downto 0);
		reg14	: out std_logic_vector(63 downto 0);
		reg15	: out std_logic_vector(63 downto 0);
		reg16	: out std_logic_vector(63 downto 0);
		reg17	: out std_logic_vector(63 downto 0);
		reg18	: out std_logic_vector(63 downto 0);
		reg19	: out std_logic_vector(63 downto 0);
		reg20	: out std_logic_vector(63 downto 0);
		reg21	: out std_logic_vector(63 downto 0);
		reg22	: out std_logic_vector(63 downto 0);
		reg23	: out std_logic_vector(63 downto 0);
		reg24	: out std_logic_vector(63 downto 0);
		reg25	: out std_logic_vector(63 downto 0);
		reg26	: out std_logic_vector(63 downto 0);
		reg27	: out std_logic_vector(63 downto 0);
		reg28	: out std_logic_vector(63 downto 0);
		reg29	: out std_logic_vector(63 downto 0);
		reg30	: out std_logic_vector(63 downto 0);
		reg31	: out std_logic_vector(63 downto 0)
	);
END regfile_ram;

ARCHITECTURE SYN OF regfile_ram IS
	TYPE ARegisters IS ARRAY (0 TO 31) OF STD_LOGIC_VECTOR (63 downto 0);
	SIGNAL Registers : ARegisters := (others => (others => '0'));

BEGIN
	reg0 <= Registers(0);
	reg1 <= Registers(1);
	reg2 <= Registers(2);
	reg3 <= Registers(3);
	reg4 <= Registers(4);
	reg5 <= Registers(5);
	reg6 <= Registers(6);
	reg7 <= Registers(7);
	reg8 <= Registers(8);
	reg9 <= Registers(9);
	reg10 <= Registers(10);
	reg11 <= Registers(11);
	reg12 <= Registers(12);
	reg13 <= Registers(13);
	reg14 <= Registers(14);
	reg15 <= Registers(15);
	reg16 <= Registers(16);
	reg17 <= Registers(17);
	reg18 <= Registers(18);
	reg19 <= Registers(19);
	reg20 <= Registers(20);
	reg21 <= Registers(21);
	reg22 <= Registers(22);
	reg23 <= Registers(23);
	reg24 <= Registers(24);
	reg25 <= Registers(25);
	reg26 <= Registers(26);
	reg27 <= Registers(27);
	reg28 <= Registers(28);
	reg29 <= Registers(29);
	reg30 <= Registers(30);
	reg31 <= Registers(31);

	process(rdclock)
	begin
		if rising_edge(rdclock) then
			if rdclocken = '1' then
				q <= Registers(to_integer(unsigned(rdaddress)));
			end if;
		end if;
	end process;

	process(wrclock)
	begin
		if rising_edge(wrclock) then
			if wrclocken = '1' then
				if wren = '1' and wraddress/="00000" then
					Registers(to_integer(unsigned(wraddress))) <= data;
				end if;
			end if;
		end if;
	end process;
end SYN;

