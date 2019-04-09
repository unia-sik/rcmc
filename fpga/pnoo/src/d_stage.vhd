library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;

entity decode_stage is
	port(
		clk          : in  std_logic;
		rst_n        : in  std_logic;
		rfo          : in  regfile_out_type; -- input from regfile (rs1, rs2)F
		rfri         : out regfile_read_in_type; -- output to regfile (raddr1, raddr2)
		csro         : in  csrfile_out_type;
		csrri        : out csrfile_read_in_type;
		d            : in  decode_in_type;
		a            : in  decode_in_type;
		q            : out decode_out_type;
		y            : out decode_out_type;
		global_stall : in  std_logic
	);

end entity decode_stage;

architecture rtl of decode_stage is
	alias decode_reg_type is decode_out_type;

	function immout(
		inst   : std_logic_vector(31 downto 0);
		opcode : std_logic_vector(6 downto 0)
		) return std_logic_vector is
		variable imm, immi, imms, immb, immu, immj, immz : std_logic_vector(63 downto 0);
	begin
		immi(63 downto 12) := (others => inst(31));
		immi(11 downto 0)  := inst(31) & inst(30 downto 25) & inst(24 downto 21) & inst(20);

		imms(63 downto 12) := (others => inst(31));
		imms(11 downto 0)  := inst(31) & inst(30 downto 25) & inst(11 downto 8) & inst(7);

		immb(63 downto 12) := (others => inst(31));
		immb(11 downto 0)  := inst(7) & inst(30 downto 25) & inst(11 downto 8) & '0';

		immu(63 downto 32) := (others => inst(31));
		immu(31 downto 12) := inst(31) & inst(30 downto 20) & inst(19 downto 12);
		immu(11 downto 0)  := (others => '0');

		immj(63 downto 20) := (others => inst(31));
		immj(19 downto 0)  := inst(19 downto 12) & inst(20) & inst(30 downto 25) & inst(24 downto 21) & '0';

		-- immediate for SYSTEM opcode
		immz(63 downto 5) := (others => '0');
		immz(4 downto 0)  := inst(19 downto 15);

		if opcode = OP_LUI or opcode = OP_AUIPC then
			imm := immu;
		elsif opcode = OP_JAL then
			imm := immj;
		elsif opcode = OP_BRANCH then
			imm := immb;
		elsif opcode = OP_FGMP then
			imm := immb;
		elsif opcode = OP_FGMP_BRANCH then
			imm := immb;
		elsif opcode = OP_STORE then
			imm := imms;
		elsif opcode = OP_SYSTEM then
			imm := immz;
		else                            -- JALR, LOAD, OP_IMM, OP_IMM32
			imm := immi;
		end if;
		return imm;
	end function immout;

	function forward(
		rdata : std_logic_vector(63 downto 0); -- register set data
		raddr : std_logic_vector(4 downto 0);
		wdata : std_logic_vector(63 downto 0); -- forwarded data
		waddr : std_logic_vector(4 downto 0);
		we    : std_logic
		) return std_logic_vector is
		variable tmp : std_logic_vector(63 downto 0);
	begin
		if we = '1' and waddr = raddr then -- forward
			tmp := wdata;
		else
			tmp := rdata;
		end if;
		return tmp;
	end function forward;

	signal r, rin : decode_reg_type;
begin
	comb : process(d, a, r, rfo, csro)
		variable v         : decode_reg_type;
		variable reg_rclke : std_logic;
		variable reg1used  : std_logic;
		variable reg2used  : std_logic;
	begin
		v := r;

		-- set read address for register file (use non registered input signals)
		rfri.raddr1 <= a.f.inst(19 downto 15);
		rfri.raddr2 <= a.f.inst(24 downto 20);

		csrri.raddr <= a.f.inst(31 downto 20);

		v.raddr1  := d.f.inst(19 downto 15);
		v.raddr2  := d.f.inst(24 downto 20);
		v.csraddr := d.f.inst(31 downto 20);

		v.pc  := d.f.pc;
		v.pc4 := d.f.pc4;

		v.opcode := d.f.inst(6 downto 0);

		v.imm := immout(d.f.inst, v.opcode);

		v.alufunc1 := d.f.inst(14 downto 12);
		v.lsfunc   := d.f.inst(14 downto 12);

		v.waddr := d.f.inst(11 downto 7);

		v.br         := '0';
		v.alu32      := '0';
		v.alufunc2   := '0';
		v.aluop1pc   := '0';
		v.aluop1zero := '0';
		v.aluop2imm  := '0';
		v.jump       := '0';
		v.jumpreg    := '0';
		v.we         := '1';
		v.dre        := '0';
		v.dwe        := '0';
		v.csrop      := '0';
		v.csrval     := csro.rdata;
		v.fgmop      := '0';

		reg1used := '1';
		reg2used := '0';

		v.exception.epc   := v.pc;
		v.exception.valid := '0';

		case v.opcode is
			when OP_LUI => v.alufunc1 := FUNC_ADD;
				v.aluop1zero := '1';
				v.aluop2imm  := '1';
				reg1used     := '0';

			when OP_AUIPC => v.alufunc1 := FUNC_ADD;
				v.aluop1pc  := '1';
				v.aluop2imm := '1';
				reg1used    := '0';

			when OP_JAL => v.alufunc1 := FUNC_ADD;
				v.aluop1pc := '1';
				v.jump     := '1';
				reg1used   := '0';

			when OP_JALR => v.alufunc1 := FUNC_ADD;
				v.jump    := '1';
				v.jumpreg := '1';

			when OP_BRANCH => v.br := '1';
				v.alufunc2 := '1';
				v.we       := '0';
				reg2used   := '1';

			when OP_LOAD => v.alufunc1 := FUNC_ADD;
				v.aluop2imm := '1';
				v.jumpreg   := '1';
				v.dre       := '1';

			when OP_STORE => v.alufunc1 := FUNC_ADD;
				v.aluop2imm := '1';
				v.jumpreg   := '1';
				v.dwe       := '1';
				v.we        := '0';
				reg2used    := '1';

			when OP_OP | OP_OP32 =>
				v.alufunc2 := d.f.inst(30);

				if v.alufunc1 = FUNC_SLT or v.alufunc1 = FUNC_SLTU then
					v.alufunc2 := '1';
				end if;

				if v.opcode = OP_OP32 then
					v.alu32 := '1';
				end if;

				reg2used := '1';

			when OP_OP_IMM | OP_OP_IMM32 =>
				v.aluop2imm := '1';
				if v.alufunc1 = FUNC_SRA then
					v.alufunc2 := d.f.inst(30);
				elsif v.alufunc1 = FUNC_SLT or v.alufunc1 = FUNC_SLTU then
					v.alufunc2 := '1';
				end if;

				if v.opcode = OP_OP_IMM32 then
					v.alu32 := '1';
				end if;

			when OP_SYSTEM =>
				v.csrop := '1';
				if v.alufunc1(2) = '1' then
					reg1used := '0';
				end if;

				if v.csraddr = CSR_SCALL then
					v.exception.cause := EXC_SYSTEM_CALL;
					v.exception.valid := '1';
				end if;

				-- jump to epc
				if v.csraddr = CSR_SRET then
					reg1used := '0';
					v.rs1      := csro.epc;
					v.alufunc1 := FUNC_ADD;
					v.aluop1pc := '0';
					v.jump     := '1';
					v.we       := '0';
					v.imm      := (others => '0');
					v.jumpreg  := '1';
				end if;

            when OP_FGMP_PIMP =>
                case v.alufunc1 is
                    when FUNC_FGMP_SND =>
                        reg2used := '1';
                        v.fgmop  := '1';
                        v.we     := '0';
                    when FUNC_FGMP_Rdy =>
                        v.fgmop := '1';
                        reg2used := '1';
                    when FUNC_FGMP_RCVN =>
                        v.fgmop := '1';
                    when FUNC_FGMP_RCVP =>
                        v.fgmop := '1';
                    when FUNC_FGMP_IBRR =>
                        reg2used := '1';
                        v.fgmop  := '1';
                        v.we     := '0';
                    when others =>
                        v.exception.cause := EXC_ILLEGAL_INSTRUCTION;
                        v.exception.valid := '1';
                end case;

            when OP_FGMP_BRANCH =>
                case v.alufunc1 is
                    when FUNC_FGMP_BSF | FUNC_FGMP_BSNF | FUNC_FGMP_BRE |
                         FUNC_FGMP_BRNE | FUNC_FGMP_BR | FUNC_FGMP_BNR |
                         FUNC_FGMP_BBRR =>
                        v.br       := '1';
                        v.alufunc2 := '1';
                        v.we       := '0';
                        reg2used   := '1';
                        v.fgmop    := '1';

                    when others => 
                        v.exception.cause := EXC_ILLEGAL_INSTRUCTION;
                        v.exception.valid := '1';
                end case;

            -- fence ops are nops
            when OP_FENCE =>
                v.we    := '0';
                reg1used := '0';

            when others =>
                v.exception.cause := EXC_ILLEGAL_INSTRUCTION;
                v.exception.valid := '1';

        end case;


		-- fetch stage caused exception
		if d.f.exception.valid = '1' then
			v.exception := d.f.exception;
		end if;

		-- do not write to register 0
		if v.waddr = "00000" then
			v.we := '0';
		end if;

		-- something up the pipe is throwing or we are jumping
		if r.jump = '1' or d.e.jump = '1' or r.exception.valid = '1' -- we are currently throwing (nop next input)
		or d.e.exception.valid = '1'    -- execute stage
		or d.m.exception.valid = '1'    -- memory stage
		or d.w.exception = '1'          -- writeback stage
		then
			v.jump            := '0';
			v.br              := '0';
			v.we              := '0';
			v.dre             := '0';
			v.dwe             := '0';
			v.csrop           := '0';
			v.fgmop           := '0';
			v.exception.valid := '0';
		end if;

		v.rs1 := rfo.data1;
		v.rs2 := rfo.data2;

		if v.stall_m = '1' then
			v := r;
		end if;

		v.rs1 := forward(v.rs1, v.raddr1, a.m.res, a.m.waddr, a.m.we);
		v.rs1 := forward(v.rs1, v.raddr1, a.e.res, a.e.waddr, a.e.we and not a.e.dre);

		v.rs2 := forward(v.rs2, v.raddr2, a.m.res, a.m.waddr, a.m.we);
		v.rs2 := forward(v.rs2, v.raddr2, a.e.res, a.e.waddr, a.e.we and not a.e.dre);

		v.csrval := csro.rdata;

		if d.m.csraddr = v.csraddr and d.m.csrwe = '1' then
			v.csrval := d.m.csrval;
		end if;

		-- if current command is SRET, forward EPC
		if v.csrop = '1' and v.csraddr = CSR_SRET and d.m.csraddr = CSR_EPC and d.m.csrwe = '1' then
			v.rs1 := d.m.csrval;
		end if;

		-- stall_m logic (wait for load command)

		-- is register 1 used and we are load to it in the prev. cycle
		if a.e.dre = '1' and a.e.waddr = v.raddr1 and reg1used = '1' then
			v.stall_m := '1';
			reg_rclke := '1';
		-- is register 2 used and we load to it in the prev cycle
		elsif a.e.dre = '1' and a.e.waddr = v.raddr2 and reg2used = '1' then
			v.stall_m := '1';
			reg_rclke := '1';
		elsif r.stall_m = '1' then
			v.stall_m := '0';
			reg_rclke := '1';
		else
			v.stall_m := '0';
			reg_rclke := '1';
		end if;

	--	-- stall_csr logic (wait until CSR op is through w stage)
	--	if v.csrop = '1' and v.csraddr /= CSR_SRET and v.stall_csr = '0' then
	--		v.csrstall  := "10";        -- stall 3 cycles
	--		v.stall_csr := '1';
	--	elsif v.stall_csr = '1' then
	--		if v.csrstall > 0 then
	--			v.stall_csr := '1';
	--			v.csrstall  := v.csrstall - 1;
	--			reg_rclke   := '1';
	--		else
	--			v.stall_csr := '0';
	--		end if;
	--	else
	--		v.stall_csr := '0';
	--	end if;

		rfri.rclke  <= reg_rclke;
		csrri.rclke <= reg_rclke;

		-- we are throwing
		if v.exception.valid = '1' then
			v.jump          := '0';
			v.br            := '0';
			v.we            := '0';
			v.dre           := '0';
			v.dwe           := '0';
			v.csrop         := '0';
			v.exception.epc := v.pc;
			v.fgmop         := '0';
		end if;

		rin <= v;

		if v.stall_m = '1' then
			v.jump  := '0';
			v.br    := '0';
			v.we    := '0';
			v.dre   := '0';
			v.dwe   := '0';
			v.csrop := '0';
			v.fgmop := '0';
		end if;

		y <= v;

		v := r;

		if r.stall_m = '1' then
			v.jump  := '0';
			v.br    := '0';
			v.we    := '0';
			v.dre   := '0';
			v.dwe   := '0';
			v.csrop := '0';
			v.fgmop := '0';
		end if;

		q <= v;

	end process;

	reg : process(clk, rst_n)           -- sequential process
	begin
		if rst_n = '0' then
			r.pc              <= x"0000000000000000";
			r.pc4             <= x"0000000000000000";
			r.jump            <= '0';
			r.jumpreg         <= '0';
			r.opcode          <= OP_OP_IMM;
			r.br              <= '0';
			r.alufunc1        <= FUNC_ADD;
			r.alufunc2        <= '0';
			r.aluop1pc        <= '0';
			r.aluop1zero      <= '0';
			r.aluop2imm       <= '1';
			r.rs1             <= (others => '0');
			r.rs2             <= (others => '0');
			r.imm             <= (others => '0');
			r.raddr1          <= (others => '0');
			r.raddr2          <= (others => '0');
			r.waddr           <= (others => '0');
			r.we              <= '0';
			r.lsfunc          <= (others => '0');
			r.dre             <= '0';
			r.dwe             <= '0';
			r.dbyteen         <= (others => '1');
			r.stall_m         <= '0';
			r.stall_csr       <= '0';
			r.alu32           <= '0';
			r.csrval          <= (others => '0');
			r.csraddr         <= (others => '0');
			r.csrop           <= '0';
			r.exception.epc   <= (others => '0');
			r.exception.cause <= (others => '0');
			r.exception.valid <= '0';
			r.csrstall        <= (others => '0');
			r.fgmop           <= '0';
		elsif rising_edge(clk) then
			if global_stall = '0' then
				r <= rin;
			end if;
		end if;
	end process;

end architecture;
