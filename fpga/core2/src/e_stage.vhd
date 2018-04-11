library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;
USE work.constants.ALL;
use work.libnode.all;

entity execute_stage is
	port(
		clk          : in  std_logic;
		rst_n        : in  std_logic;
		mcodewi      : out mcodefile_write_in_type;
		a            : in  execute_in_type;
		d            : in  execute_in_type;
		q            : out execute_out_type;
		y            : out execute_out_type;
		global_stall : in  std_logic
	);

end entity execute_stage;

architecture rtl of execute_stage is
	type execute_reg_type is record
		pc            : std_logic_vector(63 downto 0);
		address       : std_logic_vector(63 downto 0);
		jump          : std_logic;
		frm           : std_logic_vector(2 downto 0);
		fflags        : std_logic_vector(4 downto 0);
		fcsr          : std_logic_vector(31 downto 0);
		fpuop         : std_logic;
		fpumcycle     : std_logic;
		rs2           : std_logic_vector(63 downto 0);
		fprs2         : std_logic_vector(63 downto 0);
		res           : std_logic_vector(63 downto 0);
		fpres         : std_logic_vector(63 downto 0);
		waddr         : std_logic_vector(4 downto 0);
		fpwaddr       : std_logic_vector(4 downto 0);
		we            : std_logic;
		fpwe          : std_logic;
		lsfunc        : std_logic_vector(2 downto 0);
		dre           : std_logic;
		dwe           : std_logic;
		fpdre         : std_logic;
		fpdwe         : std_logic;
		dword         : std_logic;
		stall_m       : std_logic;
		extended      : std_logic;
		multiply      : std_logic;
		division      : std_logic;
		remaind       : std_logic;
		dbyteen       : std_logic_vector(7 downto 0);
		csrval        : std_logic_vector(63 downto 0);
		csraddr       : std_logic_vector(11 downto 0);
		csrwe         : std_logic;
		csrfwe        : std_logic;
		nomcode       : std_logic;
		mcode         : std_logic;
		mcodefinish   : std_logic;
		mcodewaddr1   : std_logic_vector(3 downto 0);
		mcodewaddr2   : std_logic_vector(3 downto 0);
		mcoderes1     : std_logic_vector(63 downto 0);
		mcoderes2     : std_logic_vector(63 downto 0);
		mcodewe1      : std_logic;
		mcodewe2      : std_logic;
		mcodefpwaddr  : std_logic_vector(4 downto 0);
		mcodefpres    : std_logic_vector(63 downto 0);
		mcodefpflags  : std_logic_vector(4 downto 0);
		exception     : exception_type;
		fgmop         : std_logic;
	end record;

	procedure alu(
		op1, op2 : in  std_logic_vector(63 downto 0);
		func1    : in  std_logic_vector(2 downto 0);
		func2    : in  std_logic;       -- set if srai/sub/sra or branch or slti/sltiu/slt/sltu
		alu32    : in  std_logic;
		res      : out std_logic_vector(63 downto 0);
		brcond   : out std_logic) is
		variable au : unsigned(63 downto 0); -- a unsigned
		variable bu : unsigned(63 downto 0); -- b unsigned

		variable bun : unsigned(63 downto 0); -- b unsigned negated
		variable sum : unsigned(64 downto 0);

		variable condeq  : std_logic;
		variable condlt  : std_logic;
		variable condltu : std_logic;

		variable count  : unsigned(5 downto 0);
		variable shift0 : unsigned(126 downto 0);
		variable shift5 : unsigned(63 downto 0);
	begin
		-- 64bit adder
		au := unsigned(op1);
		bu := unsigned(op2);

		bun := bu xor (63 downto 0 => func2);
		sum := ("0" & au) + ("0" & bun) + (x"0000000000000000" & func2);

		-- branch condition calculation
		if sum(63 downto 0) = to_unsigned(0, 64) then
			condeq := '1';
		else
			condeq := '0';
		end if;
		condltu := not sum(64);
		condlt  := (au(63) and bun(63)) or (au(63) and sum(63)) or (bun(63) and sum(63));

		-- shift
		if func1(2) = '0' then
			count := not bu(5 downto 0);
			if alu32 = '1' then
				count := '1' & (not bu(4 downto 0));
			end if;
			shift0(126 downto 63) := au(63 downto 0); -- SLL
			shift0(62 downto 0)   := (others => '0');
		elsif alu32 = '0' then
			count := bu(5 downto 0);
			if func2 = '1' then
				shift0(126 downto 64) := (others => au(63)); -- SRA
			else
				shift0(126 downto 64) := (others => '0'); -- SRL
			end if;
			shift0(63 downto 0) := au(63 downto 0);
		else
			count := '0' & bu(4 downto 0);
			if func2 = '1' then
				shift0(126 downto 32) := (others => au(31)); -- SRA
			else
				shift0(126 downto 32) := (others => '0'); -- SRL
			end if;
			shift0(31 downto 0) := au(31 downto 0);
		end if;

		shift5 := shift_right(shift0, to_integer(count))(63 downto 0);

		-- arithmetic logical result
		case func1 is
			when FUNC_ADD  => res := std_logic_vector(sum(63 downto 0));
			when FUNC_SLL  => res := std_logic_vector(shift5);
			when FUNC_SLT  => res := x"000000000000000" & "000" & condlt;
			when FUNC_SLTU => res := x"000000000000000" & "000" & condltu;
			when FUNC_XOR  => res := std_logic_vector(au xor bu);
			when FUNC_SRL  => res := std_logic_vector(shift5);
			when FUNC_OR   => res := std_logic_vector(au or bu);
			when FUNC_AND  => res := std_logic_vector(au and bu);
			when others    => null;
		end case;

		-- branch condition evaluation
		case func1 is
			when FUNC_BEQ  => brcond := condeq;
			when FUNC_BNE  => brcond := not condeq;
			when FUNC_BLT  => brcond := condlt;
			when FUNC_BGE  => brcond := not condlt;
			when FUNC_BLTU => brcond := condltu;
			when FUNC_BGEU => brcond := not condltu;
			when others    => null;
		end case;

	end procedure;

	function aluop1out(
		rs1     : std_logic_vector(63 downto 0);
		pc      : std_logic_vector(63 downto 0);
		op1pc   : std_logic;
		op1zero : std_logic
		) return std_logic_vector is
		variable q : std_logic_vector(63 downto 0);
	begin
		q := rs1;
		if op1pc = '1' then
			q := pc;
		end if;
		if op1zero = '1' then
			q := (others => '0');
		end if;
		return q;
	end function aluop1out;

	function aluop2out(
		rs2    : std_logic_vector(63 downto 0);
		imm    : std_logic_vector(63 downto 0);
		op2imm : std_logic
		) return std_logic_vector is
		variable q : std_logic_vector(63 downto 0);
	begin
		q := rs2;
		if op2imm = '1' then
			q := imm;
		end if;
		return q;
	end function aluop2out;

	function addresscalc(
		rs1      : std_logic_vector(63 downto 0);
		imm      : std_logic_vector(63 downto 0);
		pc       : std_logic_vector(63 downto 0);
		rs1pcsel : std_logic
		) return std_logic_vector is
		variable q : std_logic_vector(63 downto 0);
	begin
		if rs1pcsel = '1' then          -- OP_JALR, OP_LOAD, OP_STORE
			q := rs1;
		else                            -- OP_JAL, OP_BRANCH
			q := pc;
		end if;
		return (std_logic_vector(unsigned(imm) + unsigned(q)));
	end function addresscalc;

	function byteenout(
		func : std_logic_vector(1 downto 0);
		addr : std_logic_vector(2 downto 0)
		) return std_logic_vector is
		variable q : std_logic_vector(7 downto 0);
	begin
		case func is
			when "00" =>
				case addr is
					when "000" =>
						q := "00000001";
					when "001" =>
						q := "00000010";
					when "010" =>
						q := "00000100";
					when "011" =>
						q := "00001000";
					when "100" =>
						q := "00010000";
					when "101" =>
						q := "00100000";
					when "110" =>
						q := "01000000";
					when "111" =>
						q := "10000000";
					when others => null;
				end case;
			when "01" =>
				case addr(2 downto 1) is
					when "00" =>
						q := "00000011";
					when "01" =>
						q := "00001100";
					when "10" =>
						q := "00110000";
					when "11" =>
						q := "11000000";
					when others => null;
				end case;
			when "10" =>
				if addr(2) = '0' then
					q := "00001111";
				else
					q := "11110000";
				end if;
			when "11" =>
				q := "11111111";
			when others => null;
		end case;
		return q;
	end function byteenout;

	function csralu(
		rs1    : std_logic_vector(63 downto 0);
		imm    : std_logic_vector(63 downto 0);
		csrval : std_logic_vector(63 downto 0);
		immsel : std_logic;
		func   : std_logic_vector(1 downto 0)
		) return std_logic_vector is
		variable q : std_logic_vector(63 downto 0);
	begin
		if immsel = '1' then
			q := imm;
		else
			q := rs1;
		end if;

		case func is
			when FUNC_CSRRS => return (std_logic_vector(unsigned(csrval) or unsigned(q)));
			when FUNC_CSRRC => return (std_logic_vector(unsigned(csrval) and unsigned(not q)));
			when others     => return q; -- FUNC_CSRRW
		end case;
	end function csralu;

	function fdataout(
		sign   : std_logic;
		data   : std_logic_vector(63 downto 0)
		) return std_logic_vector is
		variable q  : std_logic_vector(63 downto 0);
	begin

		if sign = '1' then
			q := data;
		else
			q := x"FFFFFFFF" & data(31 downto 0);
		end if;

		return q;
	end function fdataout;

	signal r, rin : execute_reg_type;

	signal test_execute     : std_logic;
	signal test_executeb     : std_logic;
	signal test_execute2     : std_logic;
	signal test_execute3     : std_logic;
	signal test_execute4     : std_logic;
	signal test_execute5     : std_logic;
	signal test_execute6     : std_logic;
	signal test_execute7     : std_logic;
	signal test_execute8     : std_logic;
	signal test_execute9     : std_logic;

	signal test_execute_fpwe     : std_logic;
	signal test_execute_fpwaddr   : std_logic_vector(4 downto 0);


	signal test_execute_we     : std_logic;
	signal test_execute_waddr   : std_logic_vector(4 downto 0);


	signal test_execute_mcodestart     : std_logic;
	signal test_execute_mcodefpflags   : std_logic_vector(4 downto 0);

	signal multiplication_ready     : std_logic;
	signal multiplication_result    : std_logic_vector(63 downto 0);


	signal division_ready       : std_logic;
	signal remainder_result     : std_logic_vector(63 downto 0);
	signal division_result      : std_logic_vector(63 downto 0);


	signal floating_point_ready     : std_logic;
	signal floating_point_flags     : std_logic_Vector(4 downto 0);
	signal floating_point_result    : std_logic_vector(63 downto 0);

	signal floating_point_mcode_result1    : std_logic_vector(63 downto 0);
	signal floating_point_mcode_result2    : std_logic_vector(63 downto 0);

begin

	--MUL1 : Multiplication port map(d.d.rs1,d.d.rs2,d.d.extended,d.d.alu32,d.d.alufunc1,multiplication_result);

	MUL1 : MultiplicationStage port map(d.d.rs1,d.d.rs2,d.d.extended,d.d.multiply,d.d.alu32,d.d.alufunc1,rst_n,clk,
										multiplication_result,multiplication_ready);
	DIV1: Division port map(d.d.rs1,d.d.rs2,d.d.alufunc1,d.d.alu32,d.d.extended,d.d.division,d.d.remaind,rst_n,clk,
										division_result,remainder_result,division_ready);
	FPU1: FPU port map(d.d.fprs1,d.d.fprs2,d.d.fprs3,d.d.rs1,d.d.fpclass,d.d.fpuop,d.d.rm,d.d.fmt,d.d.fpufunc,d.d.fpumcycle,rst_n,clk,
                                        floating_point_flags,floating_point_result,
                                        floating_point_mcode_result1,floating_point_mcode_result2,
                                        floating_point_ready);


	comb : process(d, r, multiplication_result,division_result,remainder_result,floating_point_result,multiplication_ready,division_ready,
						floating_point_ready,floating_point_flags,floating_point_mcode_result1,floating_point_mcode_result2)
		variable v : execute_reg_type;
	begin
		v := r;

		v.fgmop := d.d.fgmop;

		v.pc := d.d.pc;

		v.exception := d.d.exception;

		v.rs2 := d.d.rs2;
		v.fprs2 := d.d.fprs2;


		v.csraddr := d.d.csraddr;
		v.fflags  := d.d.fflags;
		v.frm     := d.d.frm;
		v.fcsr    := d.d.fcsr;
		v.dword   := d.d.dword;
		v.csrwe   := '0';
		v.csrfwe  := '0';
		v.stall_m := '0';

		v.nomcode      := d.d.nomcode;
		v.mcode        := d.d.mcode;
		v.mcodefinish  := d.d.mcodefinish;
		v.mcodewaddr1  := d.d.mcodewaddr1;
		v.mcodewaddr2  := d.d.mcodewaddr2;
		v.mcodefpwaddr := d.d.mcodefpwaddr;

		if d.d.mcodestart = '1' then
			v.mcodefpres := floating_point_result;
			v.mcodefpflags := floating_point_flags;
		end if;

		test_execute_mcodestart   <= d.d.mcodestart;
		test_execute_mcodefpflags <= v.mcodefpflags;

		v.res := x"0000000000000000";
		v.fpres := x"0000000000000000";
		v.mcoderes1 := x"0000000000000000";
		v.mcoderes2 := x"0000000000000000";

		test_execute <= v.mcode;
		test_executeb <= v.mcodefinish;

		if d.d.extended = '0' and d.d.fpuop = '0' then

			alu(aluop1out(d.d.rs1, d.d.pc, d.d.aluop1pc, d.d.aluop1zero),
				aluop2out(d.d.rs2, d.d.imm, d.d.aluop2imm),
				d.d.alufunc1, d.d.alufunc2, d.d.alu32, v.res, v.jump);

		elsif d.d.extended = '1' then

			if d.d.multiply = '1' then
				v.jump := '0';
				v.stall_m := '1';
			elsif d.d.division = '1' then
				v.jump := '0';
				v.stall_m := '1';
			elsif d.d.remaind = '1' then
				v.jump := '0';
				v.stall_m := '1';
			else
				v.jump := '0';
			end if;

		elsif d.d.fpuop = '1' then

			if d.d.fpumcycle = '1' then

				v.jump := '0';
				v.stall_m := '1';

			else

				if d.d.mcodewe1 = '1' then

					v.mcoderes1 := floating_point_mcode_result1;

					if d.d.mcodewe2 = '1' then
						v.mcoderes2 := floating_point_mcode_result2;
					end if;

				elsif d.d.we = '1' then

					v.res := floating_point_result;

				elsif d.d.fpwe = '1' then

					v.fpres := floating_point_result;

				end if;

				v.fflags := floating_point_flags;

			end if;

		end if;

		if d.d.alu32 = '1' then
			v.res(63 downto 32) := (others => v.res(31));
		end if;

		if d.d.csrop = '1' and d.d.alufunc1(1 downto 0) /= FUNC_CSRNO then
			v.csrwe  := '1';
			v.csrval := csralu(d.d.rs1, d.d.imm, d.d.csrval, d.d.alufunc1(2), d.d.alufunc1(1 downto 0));
			v.res    := d.d.csrval;
	  elsif d.d.csrfop = '1' then
	        v.csrfwe  := '1';
		end if;

		v.address := addresscalc(d.d.rs1, d.d.imm, d.d.pc, d.d.jumpreg);

		if v.fgmop = '1' then
			v.res := d.d.rs1;
		end if;

		-- if JAL, JALR, or taken BRANCH
		v.jump := d.d.jump or (d.d.br and v.jump);

		if d.d.jump = '1' then
			v.res := d.d.pc4;
		end if;

		v.waddr := d.d.waddr;
		v.we    := d.d.we;

		v.fpwaddr := d.d.fpwaddr;
		v.fpwe    := d.d.fpwe;

		v.fpuop     := d.d.fpuop;
		v.fpumcycle := d.d.fpumcycle;

		v.lsfunc    := d.d.lsfunc;
		v.dre       := d.d.dre;
		v.dwe       := d.d.dwe;
		v.fpdre     := d.d.fpdre;
		v.fpdwe     := d.d.fpdwe;
		v.extended  := d.d.extended;
		v.multiply  := d.d.multiply;
		v.division  := d.d.division;
		v.remaind   := d.d.remaind;
		v.mcodewe1  := d.d.mcodewe1;
		v.mcodewe2  := d.d.mcodewe2;
		v.dbyteen   := byteenout(v.lsfunc(1 downto 0), v.address(2 downto 0));

		test_execute2 <= d.d.mcodewe1;
		test_execute3 <= d.d.mcodewe2;

		if r.stall_m = '1' and multiplication_ready = '0' and division_ready = '0' and floating_point_ready = '0' then

			v := r;

			test_execute4 <= '0';
			test_execute5 <= '0';

			test_execute6 <= '0';
			test_execute7 <= '0';


			test_execute8 <= '0';
			test_execute9 <= '0';

			test_execute_fpwe <= '0';
			test_execute_fpwaddr <= "00000";

			test_execute_we <= '0';
			test_execute_waddr <= "00000";

		elsif  multiplication_ready = '1' or division_ready = '1' or  floating_point_ready = '1' then

			v := r;


			test_execute4 <= v.mcodewe1;
			test_execute5 <= v.mcodewe2;

			test_execute6 <= v.we;
			test_execute7 <= v.fpwe;


			test_execute8 <= v.mcodefinish;
			test_execute9 <= v.fpumcycle;


			test_execute_fpwe <= v.fpwe;
			test_execute_fpwaddr <= v.fpwaddr;

			test_execute_we <= v.we;
			test_execute_waddr <= v.waddr;

			if v.multiply = '1' then
				v.res := multiplication_result;
			elsif v.division = '1' then
				v.res := division_result;
			elsif v.remaind = '1' then
				v.res := remainder_result;
			elsif v.fpumcycle = '1' then

				if d.d.mcodefinish = '1' then

					v.mcodewe1 := '0';
					v.fpwe     := '1';
					v.fpwaddr  := v.mcodefpwaddr;
					if v.mcodefpflags(4) = '1' or v.mcodefpflags(3) = '1' then
						v.fpres    := v.mcodefpres;
						v.fflags   := v.mcodefpflags;
					else
						v.fpres    := floating_point_result;
					end if;

				elsif d.d.mcode = '1' then

					if v.mcodewe1 = '1' then

						v.mcoderes1 := floating_point_mcode_result1;
						if v.mcodewe2 = '1' then
							v.mcoderes2 := floating_point_mcode_result2;
						end if;
						v.fpwe := '0';

					end if;

				elsif v.nomcode = '1'  then

					if v.fpwe = '1' then
						v.fpres    := floating_point_result;
					end if;

					if v.we = '1' then
						v.res    := floating_point_result;
					end if;

				end if;

				v.fflags := floating_point_flags;
			end if;


			v.stall_m   := '0';
			v.fpuop     := '0';
			v.fpumcycle := '0';
			v.extended  := '0';
			v.multiply  := '0';
			v.division  := '0';
			v.remaind   := '0';
		else

			test_execute4 <= '0';
			test_execute5 <= '0';

			test_execute6 <= '0';
			test_execute7 <= '0';


			test_execute8 <= '0';
			test_execute9 <= '0';
		end if;

		-- v.fpres := fdataout(v.dword,v.fpres);



		-- we are jumping or something down the pipe is throwing
		if r.jump = '1' or r.exception.valid = '1' -- we are currently throwing (nop next input)
		or d.m.exception.valid = '1'    -- memory stage
		or d.w.exception = '1'          -- writeback stage
		then
			v.jump            := '0';
			v.we              := '0';
			v.dre             := '0';
			v.dwe             := '0';
			v.fpwe            := '0';
			v.fpdre           := '0';
			v.fpdwe           := '0';
			v.csrwe           := '0';
			v.csrfwe          := '0';
			v.exception.valid := '0';
			v.fgmop           := '0';
			v.mcodewe1        := '0';
			v.mcodewe2        := '0';
		end if;

		-- we are throwing
		if v.exception.valid = '1' then
			v.jump          := '0';
			v.we            := '0';
			v.dre           := '0';
			v.dwe           := '0';
			v.fpwe          := '0';
			v.fpdre         := '0';
			v.fpdwe         := '0';
			v.csrwe         := '0';
			v.csrfwe        := '0';
			v.exception.epc := d.d.pc;
			v.fgmop         := '0';
			v.mcodewe1      := '0';
			v.mcodewe2      := '0';
		end if;

		rin <= v;

		if v.stall_m = '1' then
			v.jump      := '0';
			v.we        := '0';
			v.dre       := '0';
			v.dwe       := '0';
			v.fpwe      := '0';
			v.fpdre     := '0';
			v.fpdwe     := '0';
			v.csrwe     := '0';
			v.csrfwe    := '0';
			v.fgmop     := '0';
			v.mcodewe1  := '0';
			v.mcodewe2  := '0';
		end if;

		y.pc           <= v.pc;
		y.pcjump       <= v.address;
		y.jump         <= v.jump;
		y.rs2          <= v.rs2;
		y.fprs2        <= v.fprs2;
		y.fpuop        <= v.fpuop;
		y.fflags       <= v.fflags;
		y.frm          <= v.frm;
		y.fcsr         <= v.fcsr;
		y.fpumcycle    <= v.fpumcycle;
		y.res          <= v.res;
		y.waddr        <= v.waddr;
		y.we           <= v.we;
		y.fpres        <= v.fpres;
		y.fpwaddr      <= v.fpwaddr;
		y.fpwe         <= v.fpwe;
		y.lsfunc       <= v.lsfunc;
		y.dre          <= v.dre;
		y.fpdre        <= v.fpdre;
		y.dword        <= v.dword;
		y.stall_m      <= v.stall_m;
		y.extended     <= v.extended;
		y.multiply     <= v.multiply;
		y.division     <= v.division;
		y.remaind      <= v.remaind;
		y.dwe          <= v.dwe;
		y.fpdwe        <= v.fpdwe;
		y.dbyteen      <= v.dbyteen;
		y.csrval       <= v.csrval;
		y.csraddr      <= v.csraddr;
		y.csrwe        <= v.csrwe;
		y.csrfwe       <= v.csrfwe;
		y.mcode        <= v.mcode;
		y.mcodewe1     <= v.mcodewe1;
		y.mcodewe2     <= v.mcodewe2;
		y.mcodewaddr1  <= v.mcodewaddr1;
		y.mcodewaddr2  <= v.mcodewaddr2;
		y.mcoderes1    <= v.mcoderes1;
		y.mcoderes2    <= v.mcoderes2;
		y.exception    <= v.exception;
		y.fgmop        <= v.fgmop;

		v := r;

		if r.stall_m = '1' then
			v.jump      := '0';
			v.we        := '0';
			v.dre       := '0';
			v.dwe       := '0';
			v.fpwe      := '0';
			v.fpdre     := '0';
			v.fpdwe     := '0';
			v.csrwe     := '0';
			v.csrfwe    := '0';
			v.fgmop     := '0';
			v.mcodewe1  := '0';
			v.mcodewe2  := '0';
		end if;

		q.pc           <= v.pc;
		q.pcjump       <= v.address;
		q.rs2          <= v.rs2;
		q.fprs2        <= v.fprs2;
		q.fflags       <= v.fflags;
		q.frm          <= v.frm;
		q.fcsr         <= v.fcsr;
		q.fpuop        <= v.fpuop;
		q.fpumcycle    <= v.fpumcycle;
		q.res          <= v.res;
		q.waddr        <= v.waddr;
		q.fpres        <= v.fpres;
		q.fpwaddr      <= v.fpwaddr;
		q.lsfunc       <= v.lsfunc;
		q.dbyteen      <= v.dbyteen;
		q.csrval       <= v.csrval;
		q.csraddr      <= v.csraddr;
		q.exception    <= v.exception;
		q.jump         <= v.jump;
		q.we           <= v.we;
		q.dre          <= v.dre;
		q.fpwe         <= v.fpwe;
		q.fpdre        <= v.fpdre;
		q.dword        <= v.dword;
		q.stall_m      <= v.stall_m;
		q.extended     <= v.extended;
		q.multiply     <= v.multiply;
		q.division     <= v.division;
		q.remaind      <= v.remaind;
		q.dwe          <= v.dwe;
		q.fpdwe        <= v.fpdwe;
		q.csrwe        <= v.csrwe;
		q.csrfwe       <= v.csrfwe;
		q.mcode        <= v.mcode;
		q.mcodewe1     <= v.mcodewe1;
		q.mcodewe2     <= v.mcodewe2;
		q.mcodewaddr1  <= v.mcodewaddr1;
		q.mcodewaddr2  <= v.mcodewaddr2;
		q.mcoderes1    <= v.mcoderes1;
		q.mcoderes2    <= v.mcoderes2;
		q.fgmop        <= v.fgmop;


		-- writeback of microcode registers
		mcodewi.wclke <= '1';
		mcodewi.waddr1 <= v.mcodewaddr1;
		mcodewi.wdata1 <= v.mcoderes1;
		mcodewi.we1    <= v.mcodewe1;
		mcodewi.waddr2 <= v.mcodewaddr2;
		mcodewi.wdata2 <= v.mcoderes2;
		mcodewi.we2    <= v.mcodewe2;

--		q.pc        <= r.pc;
--		q.pcjump    <= r.address;
--		q.rs2       <= r.rs2;
--		q.res       <= r.res;
--		q.waddr     <= r.waddr;
--		q.lsfunc    <= r.lsfunc;
--		q.dbyteen   <= r.dbyteen;
--		q.csrval    <= r.csrval;
--		q.csraddr   <= r.csraddr;
--		q.exception <= r.exception;
--		q.jump      <= r.jump;
--		q.we        <= r.we;
--		q.dre       <= r.dre;
--		q.stall_m   <= r.stall_m;
--		q.extended  <= r.extended;
--		q.division  <= r.division;
--		q.remaind   <= r.remaind;
--		q.dwe       <= r.dwe;
--		q.csrwe     <= r.csrwe;
--		q.fgmop     <= r.fgmop;


	end process;

	regs : process(clk, rst_n)          -- sequential process
	begin
		if rst_n = '0' then
			r.pc              <= (others => '0');
			r.address         <= (others => '0');
			r.fflags          <= (others => '0');
			r.frm             <= (others => '0');
			r.fcsr            <= (others => '0');
			r.fpuop           <= '0';
			r.fpumcycle       <= '0';
			r.jump            <= '0';
			r.res             <= (others => '0');
			r.fpres           <= (others => '0');
			r.waddr           <= (others => '0');
			r.fpwaddr         <= (others => '0');
			r.we              <= '0';
			r.fpwe            <= '0';
			r.dword           <= '0';
			r.dre             <= '0';
			r.dwe             <= '0';
			r.fpdre           <= '0';
			r.fpdwe           <= '0';
			r.stall_m         <= '0';
			r.extended        <= '0';
			r.multiply        <= '0';
			r.division        <= '0';
			r.remaind         <= '0';
			r.csrwe           <= '0';
			r.csrfwe          <= '0';
			r.mcode           <= '0';
			r.mcodewe1        <= '0';
			r.mcodewe2        <= '0';
			r.mcodewaddr1     <= (others => '0');
			r.mcodewaddr2     <= (others => '0');
			r.mcoderes1       <= (others => '0');
			r.mcoderes2       <= (others => '0');
			r.mcodefpres      <= (others => '0');
			r.mcodefpflags    <= (others => '0');
			r.exception.epc   <= (others => '0');
			r.exception.cause <= (others => '0');
			r.exception.valid <= '0';
			r.fgmop           <= '0';
		elsif rising_edge(clk) then
			if global_stall = '0' then
				r <= rin;
			end if;
		end if;
	end process;

end architecture;
