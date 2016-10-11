LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use IEEE.math_real.all;

PACKAGE constants IS   
	--Data Bits
	CONSTANT Data_Length		: natural := 64;

	--Width(Height) of Network
	CONSTANT Dimension				: natural := 4;
	
	--Address Bits for X Direction
	CONSTANT Address_Length_X	: natural := integer(ceil(log2(real(Dimension))));
	--Address Bits for Y Direction
	CONSTANT Address_Length_Y	: natural := integer(ceil(log2(real(Dimension))));

	CONSTANT Corner_Buffer_Size : natural := 8;	
	
	--Relevant for Buffer
	CONSTANT NodeCount			: natural := Dimension*Dimension;
	CONSTANT HeapSize				: natural := 32;
	CONSTANT HeapSizeBits		: natural :=  integer(ceil(log2(real(HeapSize))));
	
	CONSTANT FifoSendBufferSize : natural := 8;
END constants;
