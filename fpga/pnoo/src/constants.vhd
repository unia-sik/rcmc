library ieee;
use ieee.std_logic_1164.all;
use IEEE.math_real.all;

package constants is
  --Data Bits
  constant Data_Length : natural := 64;
  
  --Width(Height) of Network
  constant Dimension : natural := 6;
  constant dimNoJoke : natural := Dimension;
  
  --Address Bits for X Direction
  constant Address_Length_X : natural := integer(ceil(log2(real(Dimension))));
  --Address Bits for Y Direction
  constant Address_Length_Y : natural := integer(ceil(log2(real(Dimension))));

  constant Corner_Buffer_Size : natural := 8;

  --Routing
  type Routing_Protocol is (CONF_PNOO, CONF_PNOO_DOUBLE_CORNER_BUFFER, CONF_PNOO_SORT, CONF_PNOO_EMPTY, CONF_PNOO_NO_RDY, CONF_PNOO_SRR, CONF_PNOO_DRR, CONF_PNOO_CG, CONF_PNAA, CONF_PNOA);
  constant conf_routing : Routing_Protocol := CONF_PNOO_DOUBLE_CORNER_BUFFER;
  
  --Relevant for Buffer
  constant NodeCount    : natural := Dimension*Dimension;
  constant NodeCount_Length: natural := integer(ceil(log2(real(NodeCount))));
  constant cntNoJoke    : natural := dimNoJoke*dimNoJoke;
  
  constant HeapSize : natural := 32;
  constant HeapSizeBits : natural := integer(ceil(log2(real(HeapSize))));

  constant FifoSendBufferSize : natural := 8;

  -- Constant types
  type Bypass is (CONF_BYPASS_NONE, CONF_BYPASS_UNBUF, CONF_BYPASS_BUF);
  type Stall is (CONF_STALL_CHEAP, CONF_STALL_EXP);
  type Inject is (CONF_INJECT_NONE, CONF_INJECT_REQUEST, CONF_INJECT_ALTERNATE, CONF_INJECT_THROTTLE);

  -- constants
  constant conf_bypass_y : Bypass := CONF_BYPASS_UNBUF;
  constant conf_bypass_x : Bypass := CONF_BYPASS_BUF;
  constant conf_stall_y  : Stall  := CONF_STALL_CHEAP;
  constant conf_stall_x  : Stall  := CONF_STALL_CHEAP;
  constant conf_inject_y : Inject := CONF_INJECT_NONE;
  constant conf_inject_x : Inject := CONF_INJECT_NONE;
end constants;
