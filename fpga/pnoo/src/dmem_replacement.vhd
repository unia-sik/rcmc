LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libproc.all;
use work.dmem_content.all;

ENTITY dmem IS
  PORT
  (
  Rst   : IN std_logic;    
  clk   : in  std_logic;
  ici   : in  icache_in_type;
  ico   : out icache_out_type;
  dci   : in  dcache_in_type;
  dco   : out dcache_out_type;
  clken : in  std_logic
  );
END dmem;

ARCHITECTURE SYN OF dmem IS
  SIGNAL Registers : MEM := INITA;

begin
  process(clk)
  variable i : integer;
  begin
    if clken = '1' then
      i := to_integer(unsigned(ici.addr(23 downto 2)))*4;
      ico.data <= to_stdlogicvector(
      Registers(i+3)
      &Registers(i+2)
      &Registers(i+1)
      &Registers(i)
      );
      i := to_integer(unsigned(dci.addr(23 downto 3)))*8;
      if i + 7 < 65535 then  --131072
        if dci.we = '1' then
          if dci.byteen(7) = '1' then
            Registers(i+7) <= to_bitvector(dci.wdata(63 downto 56));
          end if;
          if dci.byteen(6) = '1' then
            Registers(i+6) <= to_bitvector(dci.wdata(55 downto 48));
          end if;
          if dci.byteen(5) = '1' then
            Registers(i+5) <= to_bitvector(dci.wdata(47 downto 40));
          end if;
          if dci.byteen(4) = '1' then
            Registers(i+4) <= to_bitvector(dci.wdata(39 downto 32));
          end if;
          if dci.byteen(3) = '1' then
            Registers(i+3) <= to_bitvector(dci.wdata(31 downto 24));
          end if;
          if dci.byteen(2) = '1' then
            Registers(i+2) <= to_bitvector(dci.wdata(23 downto 16));
          end if;
          if dci.byteen(1) = '1' then
            Registers(i+1) <= to_bitvector(dci.wdata(15 downto 8));
          end if;
          if dci.byteen(0) = '1' then
            Registers(i+0) <= to_bitvector(dci.wdata(7 downto 0));
          end if;
        end if;
        dco.rdata <= to_stdlogicvector(Registers(i+7)&Registers(i+6)&Registers(i+5)&Registers(i+4)
          &Registers(i+3)&Registers(i+2)&Registers(i+1)&Registers(i+0));
      else
        dco.rdata <= (others => '0');
      end if;
    end if;
  end process;
end SYN;
