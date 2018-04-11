LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.numeric_std.all;

ENTITY fregfile_ram IS
	PORT(
		data      : IN  STD_LOGIC_VECTOR(63 DOWNTO 0);
		rdaddress : IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
		rdclock   : IN  STD_LOGIC;
		rdclocken : IN  STD_LOGIC := '1';
		wraddress : IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
		wrclock   : IN  STD_LOGIC := '1';
		wrclocken : IN  STD_LOGIC := '1';
		wren      : IN  STD_LOGIC := '0';
		dword     : IN  STD_LOGIC := '0';
		q         : OUT STD_LOGIC_VECTOR(63 DOWNTO 0);
		freg0	: out std_logic_vector(63 downto 0);
		freg1	: out std_logic_vector(63 downto 0);
		freg2	: out std_logic_vector(63 downto 0);
		freg3	: out std_logic_vector(63 downto 0);
		freg4	: out std_logic_vector(63 downto 0);
		freg5	: out std_logic_vector(63 downto 0);
		freg6	: out std_logic_vector(63 downto 0);
		freg7	: out std_logic_vector(63 downto 0);
		freg8	: out std_logic_vector(63 downto 0);
		freg9	: out std_logic_vector(63 downto 0);
		freg10	: out std_logic_vector(63 downto 0);
		freg11	: out std_logic_vector(63 downto 0);
		freg12	: out std_logic_vector(63 downto 0);
		freg13	: out std_logic_vector(63 downto 0);
		freg14	: out std_logic_vector(63 downto 0);
		freg15	: out std_logic_vector(63 downto 0);
		freg16	: out std_logic_vector(63 downto 0);
		freg17	: out std_logic_vector(63 downto 0);
		freg18	: out std_logic_vector(63 downto 0);
		freg19	: out std_logic_vector(63 downto 0);
		freg20	: out std_logic_vector(63 downto 0);
		freg21	: out std_logic_vector(63 downto 0);
		freg22	: out std_logic_vector(63 downto 0);
		freg23	: out std_logic_vector(63 downto 0);
		freg24	: out std_logic_vector(63 downto 0);
		freg25	: out std_logic_vector(63 downto 0);
		freg26	: out std_logic_vector(63 downto 0);
		freg27	: out std_logic_vector(63 downto 0);
		freg28	: out std_logic_vector(63 downto 0);
		freg29	: out std_logic_vector(63 downto 0);
		freg30	: out std_logic_vector(63 downto 0);
		freg31	: out std_logic_vector(63 downto 0)
	);
END fregfile_ram;

ARCHITECTURE SYN OF fregfile_ram IS
	TYPE FRegisters IS ARRAY (0 TO 31) OF STD_LOGIC_VECTOR (63 downto 0);
	SIGNAL Registers : FRegisters := (others => (others => '0'));

BEGIN
	freg0 <= Registers(0);
	freg1 <= Registers(1);
	freg2 <= Registers(2);
	freg3 <= Registers(3);
	freg4 <= Registers(4);
	freg5 <= Registers(5);
	freg6 <= Registers(6);
	freg7 <= Registers(7);
	freg8 <= Registers(8);
	freg9 <= Registers(9);
	freg10 <= Registers(10);
	freg11 <= Registers(11);
	freg12 <= Registers(12);
	freg13 <= Registers(13);
	freg14 <= Registers(14);
	freg15 <= Registers(15);
	freg16 <= Registers(16);
	freg17 <= Registers(17);
	freg18 <= Registers(18);
	freg19 <= Registers(19);
	freg20 <= Registers(20);
	freg21 <= Registers(21);
	freg22 <= Registers(22);
	freg23 <= Registers(23);
	freg24 <= Registers(24);
	freg25 <= Registers(25);
	freg26 <= Registers(26);
	freg27 <= Registers(27);
	freg28 <= Registers(28);
	freg29 <= Registers(29);
	freg30 <= Registers(30);
	freg31 <= Registers(31);

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
				if wren = '1' then
					Registers(to_integer(unsigned(wraddress))) <= data;
				end if;
			end if;
		end if;
	end process;
end SYN;
