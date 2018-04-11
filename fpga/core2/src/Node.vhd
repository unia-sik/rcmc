library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;
use work.constants.all;
use work.LibNode.all;

entity Node is
	port(Clk               : in  std_logic;
		   Rst               : in  std_logic;
		   NorthIn           : in  P_PORT_VERTICAL_BACK;
		   NorthOut          : out P_PORT_VERTICAL;
		   SouthIn           : in  P_PORT_VERTICAL;
		   SouthOut          : out P_PORT_VERTICAL_BACK;
		   EastIn            : IN  P_PORT_HORIZONTAL_BACK;
		   EastOut           : out P_PORT_HORIZONTAL;
		   WestIn            : in  P_PORT_HORIZONTAL;
		   WestOut           : out P_PORT_HORIZONTAL_BACK;
		   LocalOut          : out P_PORT_BUFFER;
		   LocalIn           : in  P_PORT_BUFFER;
		   BufferOverflow    : out std_logic;
		   RecvBufferFull    : in  std_logic;
		   CoreAddress       : in  Address;
		   ProcessorToBuffer : in  P_PORT_BUFFER;
		   SendBufferFull    : out std_logic
	);
end Node;

architecture Behavioral of Node is
	type PORT_BEHAVIOUR is (EMPTY, INJECT_FROM_LOCAL, FORWARD_FROM_WEST, DEFLECT_FROM_WEST, FROM_CORNER, FORWARD_FROM_SOUTH, DEFLECT_FROM_SOUTH, EJECT_FROM_SOUTH, EJECT_FROM_WEST, EJECT_FROM_CORNER, TURN_FROM_WEST);

	--FIFO for buffering address and data
	type T_FIFO is array (0 to Corner_Buffer_Size - 1) of FIFOEntry;
	signal FIFO              : T_FIFO;
	signal FIFO_TAIL_POINTER : natural;
	signal FIFO_HEAD_POINTER : natural;
	signal FIFO_EMPTY        : boolean;

	--FIFO for send buffer
	type T_SB_FIFO is array (0 to FifoSendBufferSize - 1) of std_logic_vector(((Address_Length_X + Address_Length_Y) + Data_Length) - 1 downto 0);
	signal SB_FIFO              : T_SB_FIFO;
	signal SB_FIFO_TAIL_POINTER : natural;
	signal SB_FIFO_HEAD_POINTER : natural;
	signal SB_FIFO_EMPTY        : boolean;

begin
	route : process(Clk, Rst)
		-- make sure to adjust line number in tcl to process (Clk, Rst), 57 for now

		variable north  : PORT_BEHAVIOUR;
		variable east   : PORT_BEHAVIOUR;
		variable corner : PORT_BEHAVIOUR;
		variable local  : PORT_BEHAVIOUR;

		variable stall_west  : natural;
		variable stall_east  : natural;
		variable stall_x     : natural;
		variable stall_north : natural;
		variable stall_south : natural;
		variable stall_y     : natural;
		variable local_in    : P_PORT_BUFFER;
		variable local_used  : boolean;

		variable FIFO_WR_DATA1_EN : boolean;
		variable FIFO_TAIL_TMP    : natural;
		variable FIFO_FILL_LEVEL  : natural;
		variable FIFO_ITEM_POPPED : boolean;
		variable FIFO_WR_DATA1    : FIFOEntry;

		variable SB_FIFO_ITEM_POPPED : boolean;
		variable SB_FIFO_WR_DATA1    : std_logic_vector((Address_Length_X + Address_Length_Y) + Data_Length - 1 downto 0);
		variable SB_FIFO_WR_DATA1_EN : boolean;
		variable SB_FIFO_TAIL_TMP    : natural;
		variable SB_FIFO_FILL_LEVEL  : integer;

		-- injection
		variable injx_allowed : boolean;
		variable injy_allowed : boolean;

		variable request_from_north      : boolean;
		variable request_from_east       : boolean;
		variable already_requested_south : boolean;
		variable already_requested_west  : boolean;

		variable MY_TURN_Y : boolean;
		variable MY_TURN_X : boolean;

		-- exp stall
		type stall_array_y is array (0 to Dimension - 1) of natural;
		type stall_array_x is array (0 to Dimension - 1) of natural;
		variable stall_per_y_dest : stall_array_y := (others => 0);
		variable stall_per_x_dest : stall_array_x := (others => 0);

	begin
		if RST = '0' then
			north  := EMPTY;
			east   := EMPTY;
			corner := EMPTY;
			local  := EMPTY;

			stall_west  := 0;
			stall_east  := 0;
			stall_x     := 0;
			stall_north := 0;
			stall_south := 0;
			stall_y     := 0;

			BufferOverflow <= '0';
			SendBufferFull <= '0';

			-- CB FIFO
			FIFO_TAIL_POINTER <= Corner_Buffer_Size - 1;
			FIFO_HEAD_POINTER <= 0;
			FIFO_ITEM_POPPED  := false;
			FIFO_EMPTY        <= true;
			FIFO_FILL_LEVEL   := 0;
			FIFO_WR_DATA1_EN  := false;

			-- SB FIFO
			SB_FIFO_TAIL_POINTER <= 0;
			SB_FIFO_HEAD_POINTER <= 0;
			SB_FIFO_ITEM_POPPED  := false;
			SB_FIFO_WR_DATA1     := (others => '0');
			SB_FIFO_EMPTY        <= true;
			SB_FIFO_FILL_LEVEL   := 0;
			SB_FIFO_WR_DATA1_EN  := false;

			NorthOut.DataAvailable <= '0';
			NorthOut.Marked        <= '0';
			EastOut.DataAvailable  <= '0';
			EastOut.Marked         <= '0';
			LocalOut.DataAvailable <= '0';

			WestOut.Request  <= '0';
			SouthOut.Request <= '0';

			-- stalls			
			stall_per_y_dest := (others => 0);
			stall_per_x_dest := (others => 0);

			-- injection
			request_from_north      := false;
			request_from_east       := false;
			already_requested_south := false;
			already_requested_west  := false;
			MY_TURN_Y               := false;
			MY_TURN_X               := false;

		elsif rising_edge(CLK) then

			-- Rewrite of pnconfig in VHDL.
			-- Flags are ubcc00 for now.
			-- Other flags not supported yet.

			-- FIFO should initially do nothing
			FIFO_ITEM_POPPED := false;
			FIFO_WR_DATA1_EN := false;

			SB_FIFO_ITEM_POPPED := false;
			SB_FIFO_WR_DATA1_EN := false;

			--Write to FIFO
			if ProcessorToBuffer.DataAvailable = '1' then
				SB_FIFO_WR_DATA1    := ProcessorToBuffer.Address & ProcessorToBuffer.Data;
				SB_FIFO_WR_DATA1_EN := true;
			end if;

			if SB_FIFO_TAIL_POINTER /= SB_FIFO_HEAD_POINTER then
				local_in.Address       := SB_FIFO(SB_FIFO_HEAD_POINTER)((Address_Length_X + Address_Length_Y) + Data_Length - 1 downto Data_Length);
				local_in.Data          := SB_FIFO(SB_FIFO_HEAD_POINTER)(Data_Length - 1 downto 0);
				local_in.DataAvailable := '1';
			else
				local_in.DataAvailable := '0';
				local_in.Address       := (others => '0');
				local_in.Data          := (others => '0');
			end if;

			local_used := false;

			-- Decrease all stall counters.
			if stall_south /= 0 then
				stall_south := stall_south - 1;
			end if;

			-- in_south_valid eq SouthIn.DataAvailable
			-- r->in_south_deflected eq SouthIn.Marked
			-- Y_FROM_RANK(r->in_south_fc.src) == r->y eq SouthIn.Address.AddressSender.Y = CoreAddress.Y
			if SouthIn.DataAvailable = '1' and SouthIn.Address.AddressSender.Y = CoreAddress.Y and SouthIn.Marked = '1' then
				stall_y := Dimension;
			elsif stall_y /= 0 then
				stall_y := stall_y - 1;
			end if;

			-- l136
			if stall_west /= 0 then
				stall_west := stall_west - 1;
			end if;

			-- l138
			if WestIn.DataAvailable = '1' and WestIn.Address.AddressSender.X = CoreAddress.X and WestIn.Marked = '1' then
				stall_x := Dimension;
			elsif stall_x /= 0 then
				stall_x := stall_x - 1;
			end if;

			-- stall cheap l149 pnconfig

			if conf_stall_y = CONF_STALL_EXP then
				if SouthIn.Marked = '1' then
					if stall_per_y_dest(to_integer(unsigned(SouthIn.Address.AddressReceiver.Y))) = 0 then
						stall_per_y_dest(to_integer(unsigned(SouthIn.Address.AddressReceiver.Y))) := Dimension + 1;
					end if;
				end if;
				for i in 0 to Dimension - 1 loop
					if stall_per_y_dest(i) /= 0 then
						stall_per_y_dest(i) := stall_per_y_dest(i) - 1;
					end if;
				end loop;
			else
				if stall_north /= 0 then
					stall_north := stall_north - 1;
				elsif SouthIn.Marked = '1' then
					stall_north := Dimension;
				end if;
				stall_per_y_dest := (others => 0);
			end if;

			if conf_stall_x = CONF_STALL_EXP then
				if WestIn.Marked = '1' then
					if stall_per_x_dest(to_integer(unsigned(WestIn.Address.AddressReceiver.X))) = 0 then
						stall_per_x_dest(to_integer(unsigned(WestIn.Address.AddressReceiver.X))) := Dimension + 1;
					end if;
				end if;
				for i in 0 to Dimension - 1 loop
					if stall_per_x_dest(i) /= 0 then
						stall_per_x_dest(i) := stall_per_x_dest(i) - 1;
					end if;
				end loop;
			else
				if stall_east /= 0 then
					stall_east := stall_east - 1;
				elsif WestIn.Marked = '1' then
					stall_east := Dimension;
				end if;
				stall_per_x_dest := (others => 0);
			end if;

			-- inject policy l188
			if conf_inject_y = CONF_INJECT_REQUEST and NorthIn.Request = '1' then
				request_from_north := true;
			end if;

			if conf_inject_x = CONF_INJECT_REQUEST and EastIn.Request = '1' then
				request_from_east := true;
			end if;

			if SouthIn.DataAvailable = '0' then
				already_requested_south := false;
			end if;

			if WestIn.DataAvailable = '0' then
				already_requested_west := false;
			end if;

			-- set y injection policy
			case conf_inject_y is
				when CONF_INJECT_NONE =>
					injy_allowed := true;

				when CONF_INJECT_REQUEST =>
					if MY_TURN_Y = true or request_from_north = false then
						injy_allowed := true;
					else
						injy_allowed := false;
					end if;

				when CONF_INJECT_ALTERNATE =>
				when CONF_INJECT_THROTTLE  =>
			end case;

			-- set x injection policy
			case conf_inject_x is
				when CONF_INJECT_NONE =>
					injx_allowed := true;
				when CONF_INJECT_REQUEST =>
					--if MY_TURN_X = true or request_from_east = false then
						injx_allowed := MY_TURN_X or not request_from_east;
					--else
					--	injx_allowed := false;
					--end if;
				when CONF_INJECT_ALTERNATE =>
				when CONF_INJECT_THROTTLE  =>
			end case;

			-- south to north decisions
			-- l240 pnconfig
			north := EMPTY;
			local := EMPTY;

			-- l243-257
			if SouthIn.DataAvailable = '1' then
				-- SouthIn.Address.AddressReceiver.Y /= CoreAddress.Y eq y_dest!=r->y
				if SouthIn.Address.AddressReceiver.Y /= CoreAddress.Y then
					north := FORWARD_FROM_SOUTH;
				elsif stall_south /= 0 then
					north := DEFLECT_FROM_SOUTH;
				elsif RecvBufferFull = '1' then
					stall_south := Dimension;
					north       := DEFLECT_FROM_SOUTH;
				else
					local := EJECT_FROM_SOUTH;
				end if;
			end if;

			-- corner to north if empty l260-278
			-- all conditions hardcoded, including special case for unbuffered
			--if C_bypassY /= BUF and C_bypassY /= DOUBLEBUF and FIFO_FILL_LEVEL /= 0 and FIFO(FIFO_HEAD_POINTER).Address.AddressReceiver /= CoreAddress and stall_y = 0 and stall_north = 0 and north = EMPTY then
			-- north := FROM_CORNER;
			--end if;

			if FIFO_FILL_LEVEL /= 0 and injy_allowed = true then
				if (conf_bypass_y /= CONF_BYPASS_BUF) or FIFO(FIFO_HEAD_POINTER).Address.AddressReceiver /= CoreAddress then
					if stall_y /= 0 then
						null;
					elsif (conf_stall_y = CONF_STALL_CHEAP and stall_north /= 0) or stall_per_y_dest(to_integer(unsigned(FIFO(FIFO_HEAD_POINTER).Address.AddressReceiver.Y))) /= 0 then
						null;
					elsif conf_bypass_y = CONF_BYPASS_NONE and stall_south /= 0 then
						null;
					elsif north /= EMPTY then
						null;
					else
						north := FROM_CORNER;
					end if;
				end if;
			end if;

			-- l282
			east   := EMPTY;
			corner := EMPTY;
			-- local2 := EMPTY; -- for double ejection

			-- west to east l290
			if WestIn.DataAvailable = '1' then
				if WestIn.Address.AddressReceiver.X /= CoreAddress.X then
					east := FORWARD_FROM_WEST;
				elsif stall_west /= 0 then
					east := DEFLECT_FROM_WEST;
				-- l296 case
				elsif WestIn.Address.AddressReceiver.Y = CoreAddress.Y then
					-- switch case conf_bypass_y
					case conf_bypass_y is
						when CONF_BYPASS_NONE =>
							if FIFO_FILL_LEVEL = Corner_Buffer_Size then
								stall_west := Dimension;
								east       := DEFLECT_FROM_WEST;
							else
								corner := TURN_FROM_WEST;
							end if;

						when CONF_BYPASS_UNBUF =>
							-- l298 CONF_BYPASS_UNBUF
							if RecvBufferFull = '1' or local /= EMPTY then
								stall_west := Dimension;
								east       := DEFLECT_FROM_WEST;
							else
								local := EJECT_FROM_WEST;
							end if;

						when CONF_BYPASS_BUF =>
							if FIFO_FILL_LEVEL = Corner_Buffer_Size then
								stall_west := Dimension;
								east       := DEFLECT_FROM_WEST;
							else
								corner := TURN_FROM_WEST;
							end if;

					end case;

				-- l333
				elsif FIFO_FILL_LEVEL = Corner_Buffer_Size then
					stall_west := Dimension;
					east       := DEFLECT_FROM_WEST;
				else
					corner := TURN_FROM_WEST;
				end if;
			end if;

			-- direct ejection from CB not here because only needed for buf or 2buf
			-- l.344
			if FIFO_FILL_LEVEL /= 0 and FIFO(FIFO_HEAD_POINTER).Address.AddressReceiver = CoreAddress then
				if conf_bypass_y = CONF_BYPASS_BUF then
					if RecvBufferFull = '1' then
						null;
					elsif north = FROM_CORNER then
						null;
					elsif local /= EMPTY then
						null;
					else
						local := EJECT_FROM_CORNER;
					end if;
				end if;
			end if;

			-- x bypass policy (l.362)
			if SB_FIFO_FILL_LEVEL /= 0 and local_in.DataAvailable = '1' then
				if local_in.Address(Address_Length_X + Address_Length_Y - 1 downto Address_Length_Y) = CoreAddress.X then
					case conf_bypass_x is
						
						when CONF_BYPASS_BUF =>
							if FIFO_FILL_LEVEL /= Corner_Buffer_Size and corner = EMPTY then
								corner := INJECT_FROM_LOCAL;
							end if;

						when CONF_BYPASS_NONE =>
							if injx_allowed then
								if stall_x /= 0 then
									null;
								elsif (conf_stall_x = CONF_STALL_CHEAP and stall_east /= 0) or stall_per_x_dest(to_integer(unsigned(local_in.Address(Address_Length_X + Address_Length_Y - 1 downto Address_Length_Y)))) /= 0 then
									null;
								elsif stall_west /= 0 then
									null;
								elsif east /= EMPTY then
									null;
								else
									east := INJECT_FROM_LOCAL;
								end if;
							end if;

						when CONF_BYPASS_UNBUF =>
							if injy_allowed then
								if stall_y /= 0 then
									null;
								elsif (conf_stall_y = CONF_STALL_CHEAP and stall_north /= 0) or stall_per_y_dest(to_integer(unsigned(local_in.Address(Address_Length_Y - 1 downto 0)))) /= 0 then
									null;
								elsif north /= EMPTY then
									null;
								else
									north := INJECT_FROM_LOCAL;
								end if;
							end if;

					end case;

				else
					if injx_allowed then
						if stall_x /= 0 then
							null;
						elsif (conf_stall_x = CONF_STALL_CHEAP and stall_east /= 0) or stall_per_x_dest(to_integer(unsigned(local_in.Address(Address_Length_X + Address_Length_Y - 1 downto Address_Length_Y)))) /= 0 then
							null;
						elsif east /= EMPTY then
							null;
						else
							east := INJECT_FROM_LOCAL;
						end if;
					end if;
				end if;
			end if;

			-- l414
			case north is
				when FORWARD_FROM_SOUTH =>
					NorthOut.DataAvailable <= '1';
					NorthOut.Marked        <= SouthIn.Marked;
					NorthOut.Data          <= SouthIn.Data;
					NorthOut.Address       <= SouthIn.Address;
				when DEFLECT_FROM_SOUTH =>
					NorthOut.DataAvailable <= '1';
					NorthOut.Marked        <= '1';
					NorthOut.Data          <= SouthIn.Data;
					NorthOut.Address       <= SouthIn.Address;
				when INJECT_FROM_LOCAL =>
					NorthOut.DataAvailable             <= '1';
					NorthOut.Marked                    <= '0';
					NorthOut.Data                      <= local_in.Data;
					NorthOut.Address.AddressReceiver.X <= local_in.Address(Address_Length_X + Address_Length_Y - 1 downto Address_Length_Y);
					NorthOut.Address.AddressReceiver.Y <= local_in.Address(Address_Length_Y - 1 downto 0);
					NorthOut.Address.AddressSender.X   <= CoreAddress.X;
					NorthOut.Address.AddressSender.Y   <= CoreAddress.Y;
					local_used                         := true;
					MY_TURN_Y                          := false;

				when FROM_CORNER =>
					NorthOut.DataAvailable <= '1';
					NorthOut.Marked        <= '0';
					NorthOut.Data          <= FIFO(FIFO_HEAD_POINTER).Data;
					NorthOut.Address       <= FIFO(FIFO_HEAD_POINTER).Address;
					FIFO_ITEM_POPPED       := true;
					MY_TURN_Y              := false;
				when others =>
					NorthOut.DataAvailable <= '0';
					NorthOut.Marked        <= '0';
					MY_TURN_Y              := true;
					request_from_north     := false;
			end case;

			-- l 449
			case east is
				when FORWARD_FROM_WEST =>
					EastOut.DataAvailable <= '1';
					EastOut.Marked        <= WestIn.Marked;
					EastOut.Data          <= WestIn.Data;
					EastOut.Address       <= WestIn.Address;
				when DEFLECT_FROM_WEST =>
					EastOut.DataAvailable <= '1';
					EastOut.Marked        <= '1';
					EastOut.Data          <= WestIn.Data;
					EastOut.Address       <= WestIn.Address;
				when INJECT_FROM_LOCAL =>
					EastOut.DataAvailable             <= '1';
					EastOut.Marked                    <= '0';
					EastOut.Data                      <= local_in.Data;
					EastOut.Address.AddressReceiver.X <= local_in.Address(Address_Length_X + Address_Length_Y - 1 downto Address_Length_Y);
					EastOut.Address.AddressReceiver.Y <= local_in.Address(Address_Length_Y - 1 downto 0);
					EastOut.Address.AddressSender.X   <= CoreAddress.X;
					EastOut.Address.AddressSender.Y   <= CoreAddress.Y;
					MY_TURN_X                         := false;
					local_used                        := true;
				when others =>
					EastOut.DataAvailable <= '0';
					EastOut.Marked        <= '0';
					MY_TURN_X             := true;
					request_from_east     := false;
			end case;

			-- l 476
			case local is
				when EJECT_FROM_SOUTH =>
					LocalOut.DataAvailable <= '1';
					LocalOut.Data          <= SouthIn.Data;
					LocalOut.Address       <= SouthIn.Address.AddressSender.X & SouthIn.Address.AddressSender.Y;
				when EJECT_FROM_WEST =>
					LocalOut.DataAvailable <= '1';
					LocalOut.Data          <= WestIn.Data;
					LocalOut.Address       <= WestIn.Address.AddressSender.X & WestIn.Address.AddressSender.Y;
				when EJECT_FROM_CORNER =>
					LocalOut.DataAvailable <= '1';
					LocalOut.Data          <= FIFO(FIFO_HEAD_POINTER).Data;
					LocalOut.Address       <= FIFO(FIFO_HEAD_POINTER).Address.AddressSender.X & FIFO(FIFO_HEAD_POINTER).Address.AddressSender.Y;
					FIFO_ITEM_POPPED       := true;
				when others =>
					LocalOut.DataAvailable <= '0';
			end case;

			-- TODO local2

			-- l 503
			case corner is
				when TURN_FROM_WEST =>
					FIFO_WR_DATA1.Data    := WestIn.Data;
					FIFO_WR_DATA1.Address := WestIn.Address;
					FIFO_WR_DATA1_EN      := true;
				when INJECT_FROM_LOCAL =>
					FIFO_WR_DATA1.Data                      := local_in.Data;
					FIFO_WR_DATA1.Address.AddressReceiver.X := local_in.Address(Address_Length_X + Address_Length_Y - 1 downto Address_Length_Y);
					FIFO_WR_DATA1.Address.AddressReceiver.Y := local_in.Address(Address_Length_Y - 1 downto 0);
					FIFO_WR_DATA1.Address.AddressSender.X   := CoreAddress.X;
					FIFO_WR_DATA1.Address.AddressSender.Y   := CoreAddress.Y;
					FIFO_WR_DATA1_EN                        := true;
					local_used                              := true;
				when others =>
					null;
			end case;

			if already_requested_south = false and (FIFO_FILL_LEVEL /= 0 or request_from_north = true) then
				SouthOut.Request        <= '1';
				already_requested_south := true;
			else
				SouthOut.Request <= '0';
			end if;

			if already_requested_west = false and (SB_FIFO_FILL_LEVEL /= 0 or request_from_east = true) then
				WestOut.Request        <= '1';
				already_requested_west := true;
			else
				WestOut.Request <= '0';
			end if;

			--FIFO BEGIN
			-----------------------------------------------------------------------------------
			-----------------------------------------------------------------------------------
			-- If item was removed from FIFO, move the Headpointer
			if FIFO_ITEM_POPPED then
				FIFO_HEAD_POINTER <= (FIFO_HEAD_POINTER + 1) mod Corner_Buffer_Size;
				FIFO_FILL_LEVEL   := FIFO_FILL_LEVEL - 1;
			end if;

			FIFO_TAIL_TMP := FIFO_TAIL_POINTER;

			--Insert item(s) into FIFO
			if FIFO_WR_DATA1_EN then
				FIFO_TAIL_TMP       := (FIFO_TAIL_TMP + 1) mod Corner_Buffer_size;
				FIFO(FIFO_TAIL_TMP) <= FIFO_WR_DATA1;
				FIFO_FILL_LEVEL     := FIFO_FILL_LEVEL + 1;
			end if;

			--No change to the pointer if no data was written
			FIFO_TAIL_POINTER <= FIFO_TAIL_TMP;

			--Overflow detection
			if FIFO_FILL_LEVEL >= Corner_Buffer_Size then
				BufferOverflow <= '1';
			else
				BufferOverflow <= '0';
			end if;

			--FIFO empty?
			if FIFO_FILL_LEVEL = 0 then
				FIFO_EMPTY <= true;
			else
				FIFO_EMPTY <= false;
			end if;

			--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			--FIFO END

			if SB_FIFO_TAIL_POINTER /= SB_FIFO_HEAD_POINTER and local_used then
				SB_FIFO_ITEM_POPPED := true;
			--ELSIF SB_FIFO_TAIL_POINTER = SB_FIFO_HEAD_POINTER AND ProcessorToBuffer.DataAvailable = '1' AND local_used THEN
			--    SB_FIFO_WR_DATA1_EN := FALSE;
			end if;

			--If item was removed from FIFO, move the Headpointer
			if SB_FIFO_ITEM_POPPED then
				SB_FIFO_HEAD_POINTER <= (SB_FIFO_HEAD_POINTER + 1) mod FifoSendBufferSize;
				SB_FIFO_FILL_LEVEL   := SB_FIFO_FILL_LEVEL - 1;
			end if;

			SB_FIFO_TAIL_TMP := SB_FIFO_TAIL_POINTER;

			--Insert item(s) into FIFO
			if SB_FIFO_WR_DATA1_EN and SB_FIFO_FILL_LEVEL < FifoSendBufferSize then
				SB_FIFO(SB_FIFO_TAIL_TMP) <= SB_FIFO_WR_DATA1;
				SB_FIFO_TAIL_TMP          := (SB_FIFO_TAIL_TMP + 1) mod FifoSendBufferSize;
				SB_FIFO_FILL_LEVEL        := SB_FIFO_FILL_LEVEL + 1;
			end if;

			--No change to the pointer if no data was written
			SB_FIFO_TAIL_POINTER <= SB_FIFO_TAIL_TMP;

			--SendBufferFull detection
			if SB_FIFO_FILL_LEVEL >= FifoSendBufferSize - 1 then
				SendBufferFull <= '1';
			else
				SendBufferFull <= '0';
			end if;

			--FIFO empty?
			if SB_FIFO_FILL_LEVEL = 0 then
				SB_FIFO_EMPTY <= true;
			else
				SB_FIFO_EMPTY <= false;
			end if;
		end if;
	end process;
end Behavioral;
