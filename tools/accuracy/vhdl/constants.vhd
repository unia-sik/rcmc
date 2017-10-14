library ieee;
use ieee.std_logic_1164.all;
use IEEE.math_real.all;

package constants is
  --Data Bits
  constant Data_Length : natural := 64;

  --Width(Height) of Network
  constant Dimension : natural := 4;
  --Address Bits for X Direction
  constant Address_Length_X : natural := integer(ceil(log2(real(Dimension))));
  --Address Bits for Y Direction
  constant Address_Length_Y : natural := integer(ceil(log2(real(Dimension))));

  constant Corner_Buffer_Size : natural := 8;

  --Relevant for Buffer
  constant NodeCount    : natural := Dimension*Dimension;
  constant HeapSize     : natural := 32;
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
