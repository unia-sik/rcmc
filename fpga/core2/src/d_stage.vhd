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
		fpro         : in  fregfile_out_type; -- input from fregfile (rs1, rs2, rs3)F
		fprri        : out fregfile_read_in_type; -- output to fregfile (raddr1, raddr2, raddr2)
		csro         : in  csrfile_out_type;
		csrri        : out csrfile_read_in_type;
		mcodeo       : in  mcodefile_out_type;
		mcoderi      : out mcodefile_read_in_type;
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
		elsif opcode = OP_STORE or opcode = OP_FPU_STORE then
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
		variable q : std_logic_vector(63 downto 0);
	begin
		if we = '1' and waddr = raddr then -- forward
			q := wdata;
		else
			q := rdata;
		end if;
		return q;
	end function forward;


    type amcode_limit is array (0 to 3) of std_logic_vector(7 downto 0);


    signal mcode_limit : amcode_limit := (0=>x"08",1=>x"0A",2=>x"09",3=>x"0C");

--    signal mcode_limit : amcode_limit := (0=>x"08",1=>x"0A",2=>x"0E",3=>x"11");
--    signal mcode_limit : amcode_limit := (0=>x"08",1=>x"0A",2=>x"0B",3=>x"0E");
--    signal mcode_limit : amcode_limit := (0=>x"8",1=>x"A",2=>x"B",3=>x"E");
--    signal mcode_limit : amcode_limit := (0=>x"7",1=>x"9",2=>x"A",3=>x"D");

	signal r, rin : decode_reg_type;

	signal test_decode : std_logic;
	signal test_decode2 : std_logic_vector(31 downto 0);
	signal test_decode3 : std_logic_vector(31 downto 0);
	signal test_decode4 : std_logic;


	signal test_decode5 : std_logic;
	signal test_decode6 : std_logic_vector(3 downto 0);
	signal test_decode7 : std_logic_vector(63 downto 0);
	signal test_decode8 : std_logic_vector(4 downto 0);
	signal test_decode9 : std_logic_vector(4 downto 0);
	signal test_decode10 : std_logic_vector(4 downto 0);


	signal test_decode11 : std_logic;
	signal test_decode12 : std_logic;

	signal test_decode_rm : std_logic_vector(2 downto 0);

begin
	comb : process(d, a, r, rfo, fpro, csro, mcodeo, mcode_limit)
		variable v         : decode_reg_type;
		variable reg_rclke : std_logic;
		variable reg1used  : std_logic;
		variable reg2used  : std_logic;
		variable fpreg1used  : std_logic;
		variable fpreg2used  : std_logic;
		variable fpreg3used  : std_logic;
	begin
		v := r;

		-- set read address for register file (use non registered input signals)


		test_decode2 <= d.f.inst;
		test_decode3 <= d.f.mcodeinst(to_integer(unsigned(v.mcodeindex)));

		test_decode11 <= v.mcode;
		test_decode12 <= v.mcodefinish;

		if v.mcode = '0' then

			rfri.raddr1 <= a.f.inst(19 downto 15);
			rfri.raddr2 <= a.f.inst(24 downto 20);

			fprri.raddr1 <= a.f.inst(19 downto 15);
			fprri.raddr2 <= a.f.inst(24 downto 20);
			fprri.raddr3 <= a.f.inst(31 downto 27);

			csrri.raddr <= a.f.inst(31 downto 20);

			v.inst := d.f.inst;

			v.mcode          := '0';
			v.mcodefinish    := '0';
			v.mcodeindex     := "00";
			v.mcodewe1       := '0';
			v.mcodefpwaddr   := (others => '0');
			v.mcodefrm       := (others => '0');

			v.rm             := v.inst(14 downto 12);

			test_decode <= '0';
			test_decode4 <= '0';

		else

			rfri.raddr1 <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(19 downto 15);
			rfri.raddr2 <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(24 downto 20);

			fprri.raddr1 <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(19 downto 15);
			fprri.raddr2 <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(24 downto 20);
			fprri.raddr3 <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(31 downto 27);

			csrri.raddr <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(31 downto 20);

--			v.inst := d.f.mcodeinst(to_integer(unsigned(v.mcodeindex)));

			if unsigned(d.f.mcpc(10 downto 2)) = unsigned(mcode_limit(to_integer(unsigned(v.mcodeindex)))) then

				v.inst 	         := d.f.inst;
				v.mcode          := '0';
				v.mcodefinish    := '1';
				v.mcodewe1       := '0';
				v.mcodefpwaddr   := (others => '0');
				v.mcodefrm       := (others => '0');

				v.rm             := v.inst(14 downto 12);

				test_decode4 <= '1';

			else

				v.inst := d.f.mcodeinst(to_integer(unsigned(v.mcodeindex)));

				v.rm   := v.mcodefrm;
				test_decode4 <= '0';

			end if;

		    test_decode <= '1';

		end if;

		test_decode_rm <= v.rm;

		v.opcode    := v.inst(6 downto 0);

		v.fpraddr1  := v.inst(19 downto 15);
		v.fpraddr2  := v.inst(24 downto 20);
		v.fpraddr3  := v.inst(31 downto 27);

		v.fpwaddr   := v.inst(11 downto 7);
		v.fpufunc   := v.inst(31 downto 25);
		v.fmt       := v.inst(21 downto 20);
		--v.rm        := v.inst(14 downto 12);

		v.mcodestart     := '0';
		v.mcodewaddr1    := v.inst(10 downto 7);
		v.mcodewaddr2    := (others => '0');
		v.mcodewe2       := '0';

		v.raddr1      := v.inst(19 downto 15);
		v.raddr2      := v.inst(24 downto 20);

		v.csraddr     := v.inst(31 downto 20);

		v.pc          := d.f.pc;
		v.pc4         := d.f.pc4;

		v.imm         := immout(v.inst, v.opcode);
		v.alufunc1    := v.inst(14 downto 12);
		v.lsfunc      := v.inst(14 downto 12);
		v.waddr       := v.inst(11 downto 7);


		v.frm       := csro.fcsr(7 downto 5);
		v.fflags    := csro.fcsr(4 downto 0);
		v.fcsr      := csro.fcsr;

		if v.rm = "111" then
			v.rm := v.frm;
		end if;

		v.fpumcycle  := '0';
		v.fpclass    := '0';
		v.fpuop      := '0';
		v.br         := '0';
		v.alu32      := '0';
		v.alufunc2   := '0';
		v.aluop1pc   := '0';
		v.aluop1zero := '0';
		v.aluop2imm  := '0';
		v.jump       := '0';
		v.jumpreg    := '0';
		v.we         := '1';
		v.fpwe       := '0';
		v.dre        := '0';
		v.dwe        := '0';
		v.fpdre      := '0';
		v.fpdwe      := '0';
		v.extended   := '0';
		v.multiply   := '0';
		v.division   := '0';
		v.remaind    := '0';
		v.csrop      := '0';
		v.csrfop     := '0';
		v.csrval     := csro.rdata;
		v.fgmop      := '0';
		v.dword      := '0';
		v.nomcode    := '0';

		reg1used := '1';
		reg2used := '0';

		fpreg1used := '0';
		fpreg2used := '0';
		fpreg3used := '0';

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
				v.extended := v.inst(25);

				if v.extended = '1' then

					if unsigned(v.alufunc1) < 4 then
						v.multiply := '1';
					elsif unsigned(v.alufunc1) < 6 then
						v.division := '1';
					else
						v.remaind := '1';
					end if;

				else

					v.alufunc2 := v.inst(30);

					if v.alufunc1 = FUNC_SLT or v.alufunc1 = FUNC_SLTU then
						v.alufunc2 := '1';
					end if;

				end if;

				if v.opcode = OP_OP32 then
					v.alu32 := '1';
				end if;

				reg2used := '1';

			when OP_OP_IMM | OP_OP_IMM32 =>
				v.aluop2imm := '1';
				if v.alufunc1 = FUNC_SRA then
					v.alufunc2 := v.inst(30);
				elsif v.alufunc1 = FUNC_SLT or v.alufunc1 = FUNC_SLTU then
					v.alufunc2 := '1';
				end if;

				if v.opcode = OP_OP_IMM32 then
					v.alu32 := '1';
				end if;

			when OP_FPU_LOAD => v.alufunc1 := FUNC_ADD;
				v.aluop2imm := '1';
				v.jumpreg   := '1';
				v.fpdre     := '1';
				v.fpwe      := '1';
				v.we      	:= '0';
				v.dword     := v.lsfunc(0);

			when OP_FPU_STORE => v.alufunc1 := FUNC_ADD;
				v.aluop2imm := '1';
				v.jumpreg   := '1';
				v.fpdwe     := '1';
				v.fpwe      := '0';
				v.we      	:= '0';
				fpreg2used  := '1';

			when OP_FPU_OP =>

				case v.fpufunc is
					when FUNC_FADD_S =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';
						v.nomcode   := '1';

					when FUNC_FADD_D =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';
						v.dword     := '1';
						v.nomcode   := '1';


					when FUNC_FSUB_S =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';
						v.nomcode   := '1';


					when FUNC_FSUB_D =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';
						v.dword     := '1';
						v.nomcode   := '1';


					when FUNC_FMUL_S =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';
						v.nomcode   := '1';


					when FUNC_FMUL_D =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';
						v.dword     := '1';
						v.nomcode   := '1';


					when FUNC_FDIV_S =>
						v.fpumcycle := '0';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '0';
						v.we        := '0';
						v.csrfop    := '1';

						-- MicroCode
						v.mcodestart     := '1';
						v.mcodefpwaddr   := v.inst(11 downto 7);
						v.mcodefrm       := v.inst(14 downto 12);
						v.mcode          := '1';
						v.mcodewaddr1    := "1010";
						v.mcodewaddr2    := "1011";
						v.mcodewe1       := '1';
						v.mcodewe2       := '1';
						v.mcodeindex     := "00";

					when FUNC_FDIV_D =>
						v.fpumcycle := '0';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '0';
						v.we        := '0';
						v.csrfop    := '1';
						v.dword     := '1';

						-- MicroCode
						v.mcodestart     := '1';
						v.mcodefpwaddr   := v.inst(11 downto 7);
						v.mcodefrm       := v.inst(14 downto 12);
						v.mcode          := '1';
						v.mcodewaddr1    := "1010";
						v.mcodewaddr2    := "1011";
						v.mcodewe1       := '1';
						v.mcodewe2       := '1';
						v.mcodeindex     := "01";

					when FUNC_FSQRT_S =>
						v.fpumcycle := '0';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						v.fpwe      := '0';
						v.csrfop    := '1';

						-- MicroCode
						v.mcodestart     := '1';
						v.mcodefpwaddr   := v.inst(11 downto 7);
						v.mcodefrm       := v.inst(14 downto 12);
						v.mcode          := '1';
						v.mcodewaddr1    := "1010";
						v.mcodewaddr2    := "1100";
						v.mcodewe1       := '1';
						v.mcodewe2       := '1';
						v.mcodeindex     := "10";

					when FUNC_FSQRT_D =>
						v.fpumcycle := '0';
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						v.fpwe      := '0';
						v.we        := '0';
						v.csrfop    := '1';
						v.dword     := '1';

						-- MicroCode
						v.mcodestart     := '1';
						v.mcodefpwaddr   := v.inst(11 downto 7);
						v.mcodefrm       := v.inst(14 downto 12);
						v.mcode          := '1';
						v.mcodewaddr1    := "1010";
						v.mcodewaddr2    := "1100";
						v.mcodewe1       := '1';
						v.mcodewe2       := '1';
						v.mcodeindex     := "11";

					when FUNC_FSGNJ_S =>
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';

					when FUNC_FSGNJ_D =>
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.dword     := '1';

					when FUNC_FMINMAX_S =>
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';

					when FUNC_FMINMAX_D =>
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';
						v.dword     := '1';

					when FUNC_FCOMP_S =>
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.we        := '1';
						v.csrfop    := '1';

					when FUNC_FCOMP_D =>
						v.fpuop     := '1';
						v.fmt       := v.inst(26 downto 25);
						reg1used    := '0';
						fpreg1used  := '1';
						fpreg2used  := '1';
						v.we        := '1';
						v.csrfop    := '1';
						v.dword     := '1';

					when FUNC_FCONV_D2S =>
						v.fpuop     := '1';
						reg1used    := '0';
						fpreg1used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';

					when FUNC_FCONV_S2D =>
						v.fpuop     := '1';
						reg1used    := '0';
						fpreg1used  := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.csrfop    := '1';
						v.dword     := '1';

					when FUNC_FMV_F2I_S =>

						if v.fpraddr2 = "00000" and v.alufunc1 = "000" then
							v.fpuop     := '1';
							reg1used    := '0';
							fpreg1used  := '1';
							v.we        := '1';

						elsif v.fpraddr2 = "00000" and v.alufunc1 = "001" then  --FUNC_FCLASS_S
							v.fpuop     := '1';
							reg1used    := '0';
							fpreg1used  := '1';
							v.we        := '1';
							v.fpclass   := '1';

						end if;

					when FUNC_FMV_F2I_D =>

						if v.fpraddr2 = "00000" and v.alufunc1 = "000" then
							v.fpuop     := '1';
							reg1used    := '0';
							fpreg1used  := '1';
							v.we        := '1';

						elsif v.fpraddr2 = "00000" and v.alufunc1 = "001" then
							v.fpuop     := '1';
							reg1used    := '0';
							fpreg1used  := '1';
							v.we        := '1';
							v.fpclass   := '1';

						end if;

					when FUNC_FCONV_F2I_S =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						reg1used    := '0';
						fpreg1used  := '1';
						v.we        := '1';
						v.csrfop    := '1';
						v.nomcode   := '1';


					when FUNC_FCONV_F2I_D =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						reg1used    := '0';
						fpreg1used  := '1';
						v.we        := '1';
						v.csrfop    := '1';
						v.dword     := '1';
						v.nomcode   := '1';


					when FUNC_FCONV_I2F_S =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.nomcode   := '1';


					when FUNC_FCONV_I2F_D =>
						v.fpumcycle := '1';
						v.fpuop     := '1';
						v.fpwe      := '1';
						v.we        := '0';
						v.dword     := '1';
						v.nomcode   := '1';


					when FUNC_FMV_I2F_S =>

						if v.fpraddr2 = "00000" and v.alufunc1 = "000" then
							v.fpuop     := '1';
							v.fpwe      := '1';
							v.we        := '0';

						end if;

					when FUNC_FMV_I2F_D =>

						if v.fpraddr2 = "00000" and v.alufunc1 = "000" then
							v.fpuop     := '1';
							v.fpwe      := '1';
							v.we        := '0';
							v.dword     := '1';

						end if;

					when others =>

				end case;



			when OP_FPU_MADD =>
				v.fpumcycle := '1';
				v.fpuop     := '1';
				v.fpufunc   := v.inst(6 downto 0);
				v.fmt       := v.inst(26 downto 25);
				reg1used    := '0';
				fpreg1used  := '1';
				fpreg2used  := '1';
				fpreg3used  := '1';
				v.fpwe      := '1';
				v.we        := '0';
				v.csrfop    := '1';
				v.dword     := v.fmt(0);
				v.nomcode   := '1';


			when OP_FPU_MSUB =>
				v.fpumcycle := '1';
				v.fpuop     := '1';
				v.fpufunc   := v.inst(6 downto 0);
				v.fmt       := v.inst(26 downto 25);
				reg1used    := '0';
				fpreg1used  := '1';
				fpreg2used  := '1';
				fpreg3used  := '1';
				v.fpwe      := '1';
				v.we      	:= '0';
				v.csrfop    := '1';
				v.dword     := v.fmt(0);
				v.nomcode   := '1';

			when OP_FPU_NMSUB =>
				v.fpumcycle := '1';
				v.fpuop     := '1';
				v.fpufunc   := v.inst(6 downto 0);
				v.fmt       := v.inst(26 downto 25);
				reg1used    := '0';
				fpreg1used  := '1';
				fpreg2used  := '1';
				fpreg3used  := '1';
				v.fpwe      := '1';
				v.we      	:= '0';
				v.csrfop    := '1';
				v.dword     := v.fmt(0);
				v.nomcode   := '1';

			when OP_FPU_NMADD =>
				v.fpumcycle := '1';
				v.fpuop     := '1';
				v.fpufunc   := v.inst(6 downto 0);
				v.fmt       := v.inst(26 downto 25);
				reg1used    := '0';
				fpreg1used  := '1';
				fpreg2used  := '1';
				fpreg3used  := '1';
				v.we      	:= '0';
				v.fpwe      := '1';
				v.csrfop    := '1';
				v.dword     := v.fmt(0);
				v.nomcode   := '1';

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

			when OP_FGMP =>
				case v.alufunc1 is
					when FUNC_FGMP_SEND =>
						reg2used := '1';
						v.fgmop  := '1';
						v.we     := '0';

					when FUNC_FGMP_CONG =>
						v.fgmop  := '1';

					when FUNC_FGMP_RECV =>
						v.fgmop := '1';

					when FUNC_FGMP_PROBE =>
						v.fgmop := '1';

					when FUNC_FGMP_WAIT =>
						reg1used := '0';
						v.fgmop  := '1';

					when FUNC_FGMP_ANY =>
						reg1used := '0';
						v.fgmop := '1';

					when others => v.exception.cause := EXC_ILLEGAL_INSTRUCTION;
						v.exception.valid := '1';
				end case;

			-- fence ops are nops
			when OP_FENCE =>
			v.we    := '0';
			reg1used := '0';

			when others => v.exception.cause := EXC_ILLEGAL_INSTRUCTION;
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
			v.fpwe            := '0';
			v.fpdre           := '0';
			v.fpdwe           := '0';
			v.extended        := '0';
			v.multiply        := '0';
			v.division        := '0';
			v.remaind         := '0';
			v.fpumcycle       := '0';
			v.fpuop           := '0';
			v.csrop           := '0';
			v.csrfop          := '0';
			v.fgmop           := '0';
			v.exception.valid := '0';
		end if;

		v.rs1 := rfo.data1;
		v.rs2 := rfo.data2;

		v.fprs1 := fpro.fdata1;
		v.fprs2 := fpro.fdata2;
		v.fprs3 := fpro.fdata3;

		if v.stall_m = '1' then
			v := r;
		end if;

		v.rs1 := forward(v.rs1, v.raddr1, a.m.res, a.m.waddr, a.m.we);
		v.rs1 := forward(v.rs1, v.raddr1, a.e.res, a.e.waddr, a.e.we and not a.e.dre);

		v.rs2 := forward(v.rs2, v.raddr2, a.m.res, a.m.waddr, a.m.we);
		v.rs2 := forward(v.rs2, v.raddr2, a.e.res, a.e.waddr, a.e.we and not a.e.dre);

		v.fprs1 := forward(v.fprs1, v.fpraddr1, a.m.fpres, a.m.fpwaddr, a.m.fpwe);
		v.fprs1 := forward(v.fprs1, v.fpraddr1, a.e.fpres, a.e.fpwaddr, a.e.fpwe and not a.e.fpdre);

		v.fprs2 := forward(v.fprs2, v.fpraddr2, a.m.fpres, a.m.fpwaddr, a.m.fpwe);
		v.fprs2 := forward(v.fprs2, v.fpraddr2, a.e.fpres, a.e.fpwaddr, a.e.fpwe and not a.e.fpdre);

		v.fprs3 := forward(v.fprs3, v.fpraddr3, a.m.fpres, a.m.fpwaddr, a.m.fpwe);
		v.fprs3 := forward(v.fprs3, v.fpraddr3, a.e.fpres, a.e.fpwaddr, a.e.fpwe and not a.e.fpdre);


		test_decode5 <= a.e.mcodewe1;
		test_decode6 <= a.e.mcodewaddr1;
		test_decode7 <= a.e.mcoderes1;
		test_decode8 <= v.fpraddr1;
		test_decode9 <= v.fpraddr2;
		test_decode10 <= v.fpraddr3;

		if r.mcode = '1' then
			v.fprs1 := mcodeo.rdata1;
			v.fprs1 := forward(v.fprs1, v.fpraddr1, a.e.mcoderes1, ('0' & a.e.mcodewaddr1), a.e.mcodewe1);
			v.fprs1 := forward(v.fprs1, v.fpraddr1, a.e.mcoderes2, ('0' & a.e.mcodewaddr2), a.e.mcodewe2);

			v.fprs2 := mcodeo.rdata2;
			v.fprs2 := forward(v.fprs2, v.fpraddr2, a.e.mcoderes1, ('0' & a.e.mcodewaddr1), a.e.mcodewe1);
			v.fprs2 := forward(v.fprs2, v.fpraddr2, a.e.mcoderes2, ('0' & a.e.mcodewaddr2), a.e.mcodewe2);

			v.fprs3 := mcodeo.rdata3;
			v.fprs3 := forward(v.fprs3, v.fpraddr3, a.e.mcoderes1, ('0' & a.e.mcodewaddr1), a.e.mcodewe1);
			v.fprs3 := forward(v.fprs3, v.fpraddr3, a.e.mcoderes2, ('0' & a.e.mcodewaddr2), a.e.mcodewe2);
		end if;

		v.csrval := csro.rdata;

		if d.m.csraddr = v.csraddr and v.csraddr /= FCSR_FFLAGS and d.m.csrwe = '1' then
			v.csrval := d.m.csrval;
		elsif v.csraddr = FCSR_FFLAGS then
		    if a.m.csrfwe = '1' then
		        v.csrval(63 downto 5) := (others => '0');
		        v.csrval(4 downto 0) := a.m.fflags;
		    end if;
		    if a.e.csrfwe = '1' then
		        v.csrval(63 downto 5) := (others => '0');
		        v.csrval(4 downto 0) := a.e.fflags;
		    end if;
		end if;

		-- if current command is SRET, forward EPC
		if v.csrop = '1' and v.csraddr = CSR_SRET and d.m.csraddr = CSR_EPC and d.m.csrwe = '1' then
			v.rs1 := d.m.csrval;
		end if;

		-- stall_m logic (wait for load or fgmp command)

		-- is register 1 used and we are (loading or fgmop) to it in the prev. cycle
		if (a.e.dre = '1' or (a.e.fgmop = '1' and a.e.lsfunc /= FUNC_FGMP_SEND)) and a.e.waddr = v.raddr1 and reg1used = '1' then
			v.stall_m := '1';
			reg_rclke := '1';
		-- is register 2 used and we are (loading or fgmop) to it in the prev cycle
		elsif (a.e.dre = '1' or (a.e.fgmop = '1' and a.e.lsfunc /= FUNC_FGMP_SEND)) and a.e.waddr = v.raddr2 and reg2used = '1' then
			v.stall_m := '1';
			reg_rclke := '1';
		-- is fp-register 1 used and we are (loading) to it in the prev. cycle
		elsif (a.e.fpdre = '1' and a.e.fpwaddr = v.fpraddr1 and fpreg1used = '1') then
			v.stall_m := '1';
			reg_rclke := '1';
		-- is fp-register 2 used and we are (loading) to it in the prev. cycle
		elsif (a.e.fpdre = '1' and a.e.fpwaddr = v.fpraddr2 and fpreg2used = '1') then
			v.stall_m := '1';
			reg_rclke := '1';
		-- is fp-register 3 used and we are (loading) to it in the prev. cycle
		elsif (a.e.fpdre = '1' and a.e.fpwaddr = v.fpraddr3 and fpreg3used = '1') then
			v.stall_m := '1';
			reg_rclke := '1';
		elsif a.e.multiply = '1' or a.e.division = '1' or a.e.remaind = '1' or a.e.fpumcycle = '1' then
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



		mcoderi.raddr1 <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(18 downto 15);
		mcoderi.raddr2 <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(23 downto 20);
		mcoderi.raddr3 <= a.f.mcodeinst(to_integer(unsigned(v.mcodeindex)))(30 downto 27);

		rfri.rclke    <= reg_rclke;
		fprri.rclke   <= reg_rclke;
		csrri.rclke   <= reg_rclke;
		mcoderi.rclke <= reg_rclke;

		-- we are throwing
		if v.exception.valid = '1' then
			v.jump          := '0';
			v.br            := '0';
			v.we            := '0';
			v.dre           := '0';
			v.dwe           := '0';
			v.fpwe          := '0';
			v.fpdre         := '0';
			v.fpdwe         := '0';
			v.csrop         := '0';
			v.csrfop        := '0';
			v.exception.epc := v.pc;
			v.fgmop         := '0';
			v.mcodewe1      := '0';
			v.mcodewe2      := '0';
		end if;

		rin <= v;

		if v.stall_m = '1' then
			v.jump      := '0';
			v.br        := '0';
			v.we        := '0';
			v.dre       := '0';
			v.dwe       := '0';
			v.extended  := '0';
			v.multiply  := '0';
			v.division  := '0';
			v.remaind   := '0';
			v.fpumcycle := '0';
			v.fpuop     := '0';
			v.fpwe      := '0';
			v.fpdre     := '0';
			v.fpdwe     := '0';
			v.csrop     := '0';
			v.csrfop    := '0';
			v.fgmop     := '0';
			v.mcodewe1  := '0';
			v.mcodewe2  := '0';
		end if;

		y <= v;

		v := r;

		if r.stall_m = '1' then
			v.jump      := '0';
			v.br        := '0';
			v.we        := '0';
			v.dre       := '0';
			v.dwe       := '0';
			v.extended  := '0';
			v.multiply  := '0';
			v.division  := '0';
			v.remaind   := '0';
			v.fpumcycle := '0';
			v.fpuop     := '0';
			v.fpwe      := '0';
			v.fpdre     := '0';
			v.fpdwe     := '0';
			v.csrop     := '0';
			v.csrfop    := '0';
			v.fgmop     := '0';
			v.mcodewe1  := '0';
			v.mcodewe2  := '0';
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
			r.fpufunc         <= (others => '0');
			r.fpumcycle       <= '0';
			r.fmt             <= (others => '0');
			r.frm             <= (others => '0');
			r.rm             <= (others => '0');
			r.fpuop           <= '0';
			r.fpclass         <= '0';
			r.aluop1pc        <= '0';
			r.aluop1zero      <= '0';
			r.aluop2imm       <= '1';
			r.rs1             <= (others => '0');
			r.rs2             <= (others => '0');
			r.fprs1           <= (others => '0');
			r.fprs2           <= (others => '0');
			r.fprs3           <= (others => '0');
			r.imm             <= (others => '0');
			r.raddr1          <= (others => '0');
			r.raddr2          <= (others => '0');
			r.fpraddr1        <= (others => '0');
			r.fpraddr2        <= (others => '0');
			r.fpraddr3        <= (others => '0');
			r.waddr           <= (others => '0');
			r.we              <= '0';
			r.fpwe            <= '0';
			r.dword           <= '0';
			r.lsfunc          <= (others => '0');
			r.dre             <= '0';
			r.dwe             <= '0';
			r.fpdre           <= '0';
			r.fpdwe           <= '0';
			r.dbyteen         <= (others => '1');
			r.stall_m         <= '0';
			r.stall_csr       <= '0';
			r.alu32           <= '0';
			r.extended        <= '0';
			r.multiply        <= '0';
			r.division        <= '0';
			r.remaind         <= '0';
			r.csrval          <= (others => '0');
			r.csraddr         <= (others => '0');
			r.csrop           <= '0';
			r.csrfop          <= '0';
			r.nomcode         <= '0';
			r.mcode           <= '0';
			r.mcodestart      <= '0';
			r.mcodefinish     <= '0';
			r.mcodeindex      <= (others => '0');
			r.mcodewaddr1     <= (others => '0');
			r.mcodewaddr2     <= (others => '0');
			r.mcodewe1        <= '0';
			r.mcodewe2        <= '0';
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
