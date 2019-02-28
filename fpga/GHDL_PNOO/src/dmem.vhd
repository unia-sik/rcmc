LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libproc.all;
use work.dmem_content.all;

ENTITY dmem IS
	PORT
	(
        rst   : in  std_logic;    
		clk   : in  std_logic;
		ici   : in  icache_in_type;
		ico   : out icache_out_type;
		dci   : in  dcache_in_type;
		dco   : out dcache_out_type;
		clken : in  std_logic
	);
END dmem;

ARCHITECTURE SYN OF dmem IS
	SIGNAL Registers : std_logic_vector(524287 downto 0) := INITA;

begin
	process(clk)
	begin
		if clken = '1' then

			ico.data <= Registers((524287-to_integer(unsigned(ici.addr))*8) downto ((524287-to_integer(unsigned(ici.addr))*8-31))); 
			
			if dci.addr(63 downto 24) = x"0000000000" then	-- < operator causes bound check failute in ieee.std_logic if the input is too large
				if (to_integer(unsigned(dci.addr(23 downto 3)))*64) <= (524287-63) then
					if dci.we = '1' then
						if dci.byteen(0) = '1' then
							Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-56) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-63)) <= dci.wdata(7 downto 0);
						end if;
						if dci.byteen(1) = '1' then
							Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-48) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-55)) <= dci.wdata(15 downto 8);
						end if;
						if dci.byteen(2) = '1' then
							Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-40) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-47)) <= dci.wdata(23 downto 16);
						end if;
						if dci.byteen(3) = '1' then
							Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-32) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-39)) <= dci.wdata(31 downto 24);
						end if;
						if dci.byteen(4) = '1' then
							Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-24) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-31)) <= dci.wdata(39 downto 32);
						end if;
						if dci.byteen(5) = '1' then
							Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-16) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-23)) <= dci.wdata(47 downto 40);
						end if;
						if dci.byteen(6) = '1' then
							Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-8) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-15)) <= dci.wdata(55 downto 48);
						end if;
						if dci.byteen(7) = '1' then
							Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-7)) <= dci.wdata(63 downto 56);
						end if;
					end if;
					dco.rdata <= Registers((524287-to_integer(unsigned(dci.addr(23 downto 3)))*64) downto (524287-to_integer(unsigned(dci.addr(23 downto 3)))*64-63));
				else
					dco.rdata <= x"0000000000000000";
				end if;
			else
				dco.rdata <= x"0000000000000000";
			end if;
		end if;
	end process;
end SYN;
