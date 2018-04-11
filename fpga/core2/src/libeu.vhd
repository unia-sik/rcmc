library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libproc.all;
USE work.LibNode.ALL;

package libeu is
	type exception_type is record
		epc   : std_logic_vector(63 downto 0);
		cause : std_logic_vector(4 downto 0);
		valid : std_logic;
	end record;

	type regfile_out_type is record
		data1 : std_logic_vector(63 downto 0); -- read data 1
		data2 : std_logic_vector(63 downto 0); -- read data 2
	end record;

	type fregfile_out_type is record
		fdata1 : std_logic_vector(63 downto 0); -- read fdata 1
		fdata2 : std_logic_vector(63 downto 0); -- read fdata 2
		fdata3 : std_logic_vector(63 downto 0); -- read fdata 3
	end record;

	type csrfile_out_type is record
		rdata  : std_logic_vector(63 downto 0);
		status : std_logic_vector(31 downto 0);
		epc    : std_logic_vector(63 downto 0);
		evec   : std_logic_vector(63 downto 0);
		cause  : std_logic_vector(31 downto 0);
		fcsr  : std_logic_vector(31 downto 0);
	end record;
	
	type mcodefile_out_type is record
		rdata1  : std_logic_vector(63 downto 0);
		rdata2  : std_logic_vector(63 downto 0);
		rdata3  : std_logic_vector(63 downto 0);
	end record;

	type fetch_out_type is record
		pc        : std_logic_vector(63 downto 0);
		pc4       : std_logic_vector(63 downto 0);
		mcpc      : std_logic_vector(63 downto 0);
		mcpc4     : std_logic_vector(63 downto 0);
		inst      : std_logic_vector(31 downto 0);
		mcodeinst : MCodeInstr;
		exception : exception_type;
	end record;
	
	

	type decode_out_type is record
		inst           : std_logic_vector(31 downto 0);
		pc             : std_logic_vector(63 downto 0);
		pc4            : std_logic_vector(63 downto 0);
		jump           : std_logic;
		jumpreg        : std_logic;
		opcode         : std_logic_vector(6 downto 0);
		br             : std_logic;
		alufunc1       : std_logic_vector(2 downto 0);
		alufunc2       : std_logic;
		fpufunc        : std_logic_vector(6 downto 0);
		fpumcycle      : std_logic;
		fmt            : std_logic_vector(1 downto 0);
		rm             : std_logic_vector(2 downto 0);
		frm            : std_logic_vector(2 downto 0);
		fflags         : std_logic_vector(4 downto 0);
		fcsr           : std_logic_vector(31 downto 0);
		fpuop          : std_logic;
		fpclass        : std_logic;
		aluop1pc       : std_logic;
		aluop1zero     : std_logic;
		aluop2imm      : std_logic;
		rs1            : std_logic_vector(63 downto 0);
		rs2            : std_logic_vector(63 downto 0);
		fprs1          : std_logic_vector(63 downto 0);
		fprs2          : std_logic_vector(63 downto 0);
		fprs3          : std_logic_vector(63 downto 0);
		imm            : std_logic_vector(63 downto 0);
		raddr1         : std_logic_vector(4 downto 0);
		raddr2         : std_logic_vector(4 downto 0);
		fpraddr1       : std_logic_vector(4 downto 0);
		fpraddr2       : std_logic_vector(4 downto 0);
		fpraddr3       : std_logic_vector(4 downto 0);
		waddr          : std_logic_vector(4 downto 0);
		fpwaddr        : std_logic_vector(4 downto 0);
		we             : std_logic;
		fpwe           : std_logic;
		lsfunc         : std_logic_vector(2 downto 0);
		dre            : std_logic;
		dwe            : std_logic;
		fpdre          : std_logic;
		fpdwe          : std_logic;
		dword          : std_logic;
		dbyteen        : std_logic_vector(7 downto 0);
		stall_m        : std_logic;
		stall_csr      : std_logic;
		csrstall       : unsigned(1 downto 0);
		alu32          : std_logic;
		extended       : std_logic;
		multiply       : std_logic;
		division       : std_logic;
		remaind        : std_logic;
		csrop          : std_logic;
		csrfop         : std_logic;
		csrval         : std_logic_vector(63 downto 0);
		csraddr        : std_logic_vector(11 downto 0);
		nomcode        : std_logic;
		mcode          : std_logic;
		mcodestart     : std_logic;
		mcodefinish    : std_logic;
		mcodewaddr1    : std_logic_Vector(3 downto 0);
		mcodewaddr2    : std_logic_Vector(3 downto 0);
		mcodewe1       : std_logic;
		mcodewe2       : std_logic;
		mcodeindex     : std_logic_vector(1 downto 0);
		mcodefpwaddr   : std_logic_vector(4 downto 0);
		mcodefrm       : std_logic_vector(2 downto 0);
		exception      : exception_type;
		fgmop          : std_logic;
	end record;

	type execute_out_type is record
		pc            : std_logic_vector(63 downto 0);
		pcjump        : std_logic_vector(63 downto 0);
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
		mcode         : std_logic;
		mcodewaddr1   : std_logic_Vector(3 downto 0);
		mcodewaddr2   : std_logic_Vector(3 downto 0);
		mcodewe1      : std_logic;
		mcodewe2      : std_logic;
		mcoderes1     : std_logic_vector(63 downto 0);
		mcoderes2     : std_logic_vector(63 downto 0);
		exception     : exception_type;
		fgmop         : std_logic;
	end record;

	type memory_out_type is record
		pc        : std_logic_vector(63 downto 0);
		res       : std_logic_vector(63 downto 0);
        fpuop     : std_logic;
		frm       : std_logic_vector(2 downto 0);
		fflags    : std_logic_vector(4 downto 0);
		fcsr      : std_logic_vector(31 downto 0);
		fpres     : std_logic_vector(63 downto 0);
		waddr     : std_logic_vector(4 downto 0);
		fpwaddr   : std_logic_vector(4 downto 0);
		we        : std_logic;
		fpwe      : std_logic;
		dword     : std_logic;
		csrval    : std_logic_vector(63 downto 0);
		csraddr   : std_logic_vector(11 downto 0);
		csrwe     : std_logic;
		csrfwe    : std_logic;
		mcode     : std_logic;
		exception : exception_type;
	end record;

	type writeback_out_type is record
		exception : std_logic;
	end record;

	type regfile_read_in_type is record
		rclke  : std_logic;             -- clock enable
		raddr1 : std_logic_vector(4 downto 0); -- read address 1
		raddr2 : std_logic_vector(4 downto 0); -- read address 2
	end record;

	type fregfile_read_in_type is record
		rclke  : std_logic;             -- clock enable
		raddr1 : std_logic_vector(4 downto 0); -- read address 1
		raddr2 : std_logic_vector(4 downto 0); -- read address 2
		raddr3 : std_logic_vector(4 downto 0); -- read address 3
	end record;

	type regfile_write_in_type is record
		wclke : std_logic;              -- clock enable
		waddr : std_logic_vector(4 downto 0); -- write address
		wdata : std_logic_vector(63 downto 0); -- write data
		we    : std_logic;
	end record;

	type fregfile_write_in_type is record
		wclke : std_logic;              -- clock enable
		waddr : std_logic_vector(4 downto 0); -- write address
		wdata : std_logic_vector(63 downto 0); -- write data
		we    : std_logic;
		dword : std_logic;                      -- doubleword
	end record;

	type csrfile_read_in_type is record
		rclke : std_logic;              -- clock enable
		raddr : std_logic_vector(11 downto 0); -- read address
	end record;
	
	type mcodefile_read_in_type is record
		rclke    : std_logic;                      -- clock enable
		start    : std_logic;                      -- start enable
		rcindex  : std_logic_vector(1 downto 0);   -- command index
		raddr1   : std_logic_vector(3 downto 0);   -- read address 1
		raddr2   : std_logic_vector(3 downto 0);   -- read address 2
		raddr3   : std_logic_vector(3 downto 0);   -- read address 3
	end record;

	type csrfile_write_in_type is record
		wclke  : std_logic;              -- clock enable
		waddr  : std_logic_vector(11 downto 0); -- write address
		wdata  : std_logic_vector(63 downto 0); -- write data
		we     : std_logic;

		cause  : std_logic_vector(4 downto 0);
		epc    : std_logic_vector(63 downto 0);
		ewe    : std_logic;
        
		fflags : std_logic_vector(4 downto 0);
		fwe    : std_logic;
	end record;

	type  mcodefile_write_in_type is record
		wclke  : std_logic;              -- clock enable
		waddr1  : std_logic_vector(3 downto 0); -- write address 1
		waddr2  : std_logic_vector(3 downto 0); -- write address 2
		wdata1  : std_logic_vector(63 downto 0); -- write data 1
		wdata2  : std_logic_vector(63 downto 0); -- write data 2
		we1     : std_logic;
		we2     : std_logic;
	end record;

	type fetch_in_type is record
		d : decode_out_type;
		e : execute_out_type;
		m : memory_out_type;
		w : writeback_out_type;
	end record;

	type decode_in_type is record
		f : fetch_out_type;
		--
		e : execute_out_type;
		m : memory_out_type;
		w : writeback_out_type;
	end record;

	type execute_in_type is record
		d : decode_out_type;
		--
		m : memory_out_type;
		w : writeback_out_type;
	end record;

	type memory_in_type is record
		e : execute_out_type;
		--
		w : writeback_out_type;
	end record;

	type writeback_in_type is record
		m : memory_out_type;
	end record;

	component regfile
		port(
			clk          : in  std_logic;
			rst_n        : in  std_logic;
			rfri         : in  regfile_read_in_type;
			rfwi         : in  regfile_write_in_type;
			rfo          : out regfile_out_type;
			global_stall : in  std_logic
		);
	end component regfile;

	component fregfile
		port(
			clk          : in  std_logic;
			rst_n        : in  std_logic;
			fprri        : in  fregfile_read_in_type;
			fprwi        : in  fregfile_write_in_type;
			fpro         : out fregfile_out_type;
			global_stall : in  std_logic
		);
	end component fregfile;

	component csrfile
		generic(
			id    : integer;
			count : integer;
			nocdim : std_logic_vector(63 downto 0)
		);
		port(
			clk          : in  std_logic;
			rst_n        : in  std_logic;
			csrri        : in  csrfile_read_in_type;
			csrwi        : in  csrfile_write_in_type;
			csro         : out csrfile_out_type;
			global_stall : in  std_logic
		);
	end component csrfile;
	
	
	component mcodefile is
		port(
			clk          : in  std_logic;
			rst_n        : in  std_logic;
			mcoderi      : in  mcodefile_read_in_type;
			mcodewi      : in  mcodefile_write_in_type;
			mcodeo       : out mcodefile_out_type;
			global_stall : in  std_logic
		);
	end component mcodefile;

	component fetch_stage
		port(
			clk          : in  std_logic;
			rst_n        : in  std_logic;
			csro         : in  csrfile_out_type;
			ico          : in  icache_out_type;
			ici          : out icache_in_type;
			mcco         : in  mcodecache_out_type;
			mcci         : out mcodecache_in_type;
			d            : in  fetch_in_type;
			a            : in  fetch_in_type;
			q            : out fetch_out_type;
			y            : out fetch_out_type;
			global_stall : in  std_logic
		);
	end component fetch_stage;

	component decode_stage
		port(
			clk          : in  std_logic;
			rst_n        : in  std_logic;
			rfo          : in  regfile_out_type;
			rfri         : out regfile_read_in_type;
			fpro         : in  fregfile_out_type;
			fprri        : out fregfile_read_in_type;
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
	end component decode_stage;

	component execute_stage
		port(
			clk          : in  std_logic;
			rst_n        : in  std_logic;
			mcodewi      : out mcodefile_write_in_type;
			d            : in  execute_in_type;
			a            : in  execute_in_type;
			q            : out execute_out_type;
			y            : out execute_out_type;
			global_stall : in  std_logic
		);
	end component execute_stage;

	component memory_stage
		port(
			clk              : in  std_logic;
			rst_n            : in  std_logic;
			dco              : in  dcache_out_type;
			dci              : out dcache_in_type;
			d                : in  memory_in_type;
			a                : in  memory_in_type;
			q                : out memory_out_type;
			y                : out memory_out_type;
			global_stall     : in  std_logic;
			global_stall_out : out std_logic;
			nbi              : in  P_PORT_BUFFER;
			nbo              : out P_PORT_BUFFER;
			nbr              : out REQUEST_PORT;
			nbsf             : in  std_logic
		);
	end component memory_stage;

	component writeback_stage
		port(
			clk          : in  std_logic;
			rst_n        : in  std_logic;
			rfwi         : out regfile_write_in_type;
			fprwi        : out fregfile_write_in_type;
			csrwi        : out csrfile_write_in_type;
			d            : in  writeback_in_type;
			a            : in  writeback_in_type;
			q            : out writeback_out_type;
			y            : out writeback_out_type;
			global_stall : in  std_logic
		);
	end component writeback_stage;
	
	
	
	component Multiplication
		port(
			multiplicand_0  : in  std_logic_vector(63 downto 0);
			multiplicand_1  : in  std_logic_vector(63 downto 0);
			extended        : in  std_logic;
			alu32           : in  std_logic;
			funct           : in  std_logic_vector(2 downto 0);
			result          : out std_logic_vector(63 downto 0)
		);
	end component Multiplication;
	
	
	
	component MultiplicationStage is
		Port(
			multiplicand_0      : in  std_logic_vector(63 downto 0);
			multiplicand_1      : in  std_logic_vector(63 downto 0);
			extended            : in  std_logic;
			multiply            : in  std_logic;
			alu32               : in  std_logic;
			funct               : in  std_logic_vector(2 downto 0);
			nRST                : in  std_logic;
			CLK                 : in  std_logic;
			result              : out std_logic_vector(63 downto 0);
			ready               : out std_logic
		);
	end component MultiplicationStage;
	
	
	component Division 
		port(
			dividend    : in  std_logic_vector(63 downto 0);
			divisor     : in  std_logic_vector(63 downto 0);
			funct       : in  std_logic_vector(2 downto 0);
			alu32       : in  std_logic;
			extended    : in  std_logic;
			division    : in  std_logic;
			remaind     : in  std_logic;
			nRST        : in  std_logic;
			CLK         : in  std_logic;
			quotient    : out std_logic_vector(63 downto 0);
			remainder   : out std_logic_vector(63 downto 0);
			ready       : out std_logic
		);
	end component Division;
	
	
	
	component FPUCLASS
		port ( 
			A       : in std_logic_vector(63 downto 0);
			D       : in std_logic_vector(63 downto 0);
			FMT     : in std_logic_vector(1 downto 0);
			Funct   : in std_logic_vector(6 downto 0);
			RM      : in std_logic_vector(2 downto 0);
			AuxI    : in std_logic;
			
----------------------------------------------------------
			Ex      : out std_logic_Vector(4 downto 0);
			P       : out std_logic_vector(63 downto 0)
		);			
	end component;


	component FPUSIGNJ
		port ( 
			A       : in std_logic_vector(63 downto 0);
			B       : in std_logic_vector(63 downto 0);
			Funct   : in std_logic_vector(6 downto 0);
			RM      : in std_logic_vector(2 downto 0);
			
----------------------------------------------------------
			Ex      : out std_logic_Vector(4 downto 0);
			P       : out std_logic_vector(63 downto 0)
		);
	end component;


	component FPUMINMAX
		port ( 
			A       : in std_logic_vector(63 downto 0);
			B       : in std_logic_vector(63 downto 0);
			Funct   : in std_logic_vector(6 downto 0);
			RM      : in std_logic_vector(2 downto 0);
			
----------------------------------------------------------
			Ex      : out std_logic_Vector(4 downto 0);
			P       : out std_logic_vector(63 downto 0)
		);
	end component;


	component FPUCOMP
		port ( 
			A       : in std_logic_vector(63 downto 0);
			B       : in std_logic_vector(63 downto 0);
			Funct   : in std_logic_vector(6 downto 0);
			RM      : in std_logic_vector(2 downto 0);
			
----------------------------------------------------------
			Ex      : out std_logic_Vector(4 downto 0);
			P       : out std_logic_vector(63 downto 0)
		);
	end component;

	component FPUtoINT
		port (
			FPU     : in std_logic_vector(63 downto 0);
			FMT     : in std_logic_vector(1 downto 0);
			Funct   : in std_logic_vector(6 downto 0);
			RM      : in std_logic_vector(2 downto 0);
			FPUOP   : in  std_logic;
			nRST    : in std_logic;
			CLK     : in  std_logic;
			
----------------------------------------------------------
			Ex      : out std_logic_Vector(4 downto 0);
			INT     : out std_logic_vector(63 downto 0);
			Ready   : out  std_logic
		);
	end component;


	component INTtoFPU
		port ( 
			INT     : in std_logic_vector(63 downto 0);
			FMT     : in std_logic_vector(1 downto 0);
			Funct   : in std_logic_vector(6 downto 0);
			RM      : in std_logic_vector(2 downto 0);
			FPUOP   : in  std_logic;
			nRST    : in std_logic;
			CLK     : in  std_logic;
			
----------------------------------------------------------
			Ex      : out std_logic_Vector(4 downto 0);
			FPU     : out std_logic_vector(63 downto 0);
			Ready   : out  std_logic
		);
	end component;


	component FPUtoFPU
		port ( 
			FPUI    : in std_logic_vector(63 downto 0);
			FMT     : in std_logic_vector(1 downto 0);
			Funct   : in std_logic_vector(6 downto 0);
			RM      : in std_logic_vector(2 downto 0);
			
----------------------------------------------------------
			Ex      : out std_logic_Vector(4 downto 0);
			FPUO    : out std_logic_vector(63 downto 0)
		);
	end component;


	component FPUFMA
		port (
			A       : in  std_logic_vector(63 downto 0);
			B       : in  std_logic_vector(63 downto 0);
			C       : in  std_logic_vector(63 downto 0);
			FMT     : in  std_logic_vector(1 downto 0);
			Funct   : in  std_logic_vector(6 downto 0);
			RM      : in  std_logic_vector(2 downto 0);
			FPUOP   : in  std_logic;
			nRST    : in std_logic;
			CLK     : in  std_logic;
			
----------------------------------------------------------
			Ex      : out std_logic_Vector(4 downto 0);
			P       : out std_logic_vector(63 downto 0);
			Ready   : out std_logic
		);
	end component;

	component FPUDIV
		port ( 
			A : in std_logic_vector(63 downto 0);
			B : in std_logic_vector(63 downto 0);
			FMT : in std_logic_vector(1 downto 0);
			Funct : in std_logic_vector(6 downto 0);
			RM : in std_logic_vector(2 downto 0);
			
----------------------------------------------------------
			Ex : out std_logic_Vector(4 downto 0);
			P : out std_logic_vector(63 downto 0);
			PA : out std_logic_vector(63 downto 0);
			PB : out std_logic_vector(63 downto 0)
		);
	end component;

	component FPUSQRT
		Port ( 
			A : in std_logic_vector(63 downto 0);
			FMT : in std_logic_vector(1 downto 0);
			Funct : in std_logic_vector(6 downto 0);
			RM : in std_logic_vector(2 downto 0);
			
----------------------------------------------------------
			Ex : out std_logic_Vector(4 downto 0);
			P : out std_logic_vector(63 downto 0);
			PA : out std_logic_vector(63 downto 0);
			I : out std_logic_vector(63 downto 0)
		);
	end component;

	component FPU 
		port ( 
			A           : in std_logic_vector(63 downto 0);
			B           : in std_logic_vector(63 downto 0);
			C           : in std_logic_vector(63 downto 0);
			D           : in std_logic_vector(63 downto 0);
			AuxI        : in std_logic;
			FPUOPI      : in std_logic;
			RMI         : in std_logic_vector(2 downto 0);
			FMT         : in std_logic_vector(1 downto 0);
			Funct       : in std_logic_vector(6 downto 0);
			FPUCyclesI  : in std_logic;
			nRST        : in std_logic;
			CLK         : in  std_logic;
			
----------------------------------------------------------
			Ex          : out std_logic_Vector(4 downto 0);
			P           : out std_logic_vector(63 downto 0);
			M1          : out std_logic_vector(63 downto 0);
			M2          : out std_logic_vector(63 downto 0);
			Ready       : out std_logic
		);
	end component;


	component Multiplexer
		port(
			sel     : in std_logic;
			data0   : in std_logic;
			data1   : in std_logic;
			result  : out std_logic
		);
	end component;


	constant OP_LUI      : std_logic_vector(6 downto 0) := "0110111";
	constant OP_AUIPC    : std_logic_vector(6 downto 0) := "0010111";
	constant OP_JAL      : std_logic_vector(6 downto 0) := "1101111";
	constant OP_JALR     : std_logic_vector(6 downto 0) := "1100111";
	constant OP_BRANCH   : std_logic_vector(6 downto 0) := "1100011";
	constant OP_LOAD     : std_logic_vector(6 downto 0) := "0000011";
	constant OP_STORE    : std_logic_vector(6 downto 0) := "0100011";
	constant OP_OP_IMM   : std_logic_vector(6 downto 0) := "0010011";
	constant OP_OP       : std_logic_vector(6 downto 0) := "0110011";
	constant OP_OP_IMM32 : std_logic_vector(6 downto 0) := "0011011";
	constant OP_OP32     : std_logic_vector(6 downto 0) := "0111011";
	constant OP_SYSTEM   : std_logic_vector(6 downto 0) := "1110011";
	constant OP_FGMP     : std_logic_vector(6 downto 0) := "1101011";
	constant OP_FENCE    : std_logic_vector(6 downto 0) := "0001111";
	constant OP_FPU_LOAD : std_logic_vector(6 downto 0) := "0000111";
	constant OP_FPU_STORE: std_logic_vector(6 downto 0) := "0100111";
	constant OP_FPU_OP   : std_logic_vector(6 downto 0) := "1010011";
	constant OP_FPU_MADD : std_logic_vector(6 downto 0) := "1000011";
	constant OP_FPU_MSUB : std_logic_vector(6 downto 0) := "1000111";
	constant OP_FPU_NMSUB: std_logic_vector(6 downto 0) := "1001011";
	constant OP_FPU_NMADD: std_logic_vector(6 downto 0) := "1001111";

	constant FUNC_BEQ  : std_logic_vector(2 downto 0) := "000";
	constant FUNC_BNE  : std_logic_vector(2 downto 0) := "001";
	constant FUNC_BLT  : std_logic_vector(2 downto 0) := "100";
	constant FUNC_BGE  : std_logic_vector(2 downto 0) := "101";
	constant FUNC_BLTU : std_logic_vector(2 downto 0) := "110";
	constant FUNC_BGEU : std_logic_vector(2 downto 0) := "111";
	
	
	constant FUNC_MUL      : std_logic_vector(2 downto 0) := "000";
	constant FUNC_MULH     : std_logic_vector(2 downto 0) := "001";
	constant FUNC_MULHSU   : std_logic_vector(2 downto 0) := "010";
	constant FUNC_MULHU    : std_logic_vector(2 downto 0) := "011";
	constant FUNC_DIV      : std_logic_vector(2 downto 0) := "100";
	constant FUNC_DIVU     : std_logic_vector(2 downto 0) := "101";
	constant FUNC_REM      : std_logic_vector(2 downto 0) := "110";
	constant FUNC_REMU     : std_logic_vector(2 downto 0) := "111";

	constant FUNC_ADD  : std_logic_vector(2 downto 0) := "000";
	constant FUNC_SUB  : std_logic_vector(2 downto 0) := "000";
	constant FUNC_SLL  : std_logic_vector(2 downto 0) := "001";
	constant FUNC_SLT  : std_logic_vector(2 downto 0) := "010";
	constant FUNC_SLTU : std_logic_vector(2 downto 0) := "011";
	constant FUNC_XOR  : std_logic_vector(2 downto 0) := "100";
	constant FUNC_SRL  : std_logic_vector(2 downto 0) := "101";
	constant FUNC_SRA  : std_logic_vector(2 downto 0) := "101";
	constant FUNC_OR   : std_logic_vector(2 downto 0) := "110";
	constant FUNC_AND  : std_logic_vector(2 downto 0) := "111";

	constant FUNC_FENCE   : std_logic_vector(2 downto 0) := "000";
	constant FUNC_FENCE_I : std_logic_vector(2 downto 0) := "001";

	constant FUNC_CSRNO : std_logic_vector(1 downto 0) := "00"; -- No Operation
	constant FUNC_CSRRW : std_logic_vector(1 downto 0) := "01";
	constant FUNC_CSRRS : std_logic_vector(1 downto 0) := "10";
	constant FUNC_CSRRC : std_logic_vector(1 downto 0) := "11";
	
	
	
	constant FUNC_FADD_S        : std_logic_vector(6 downto 0) := "0000000";
	constant FUNC_FSUB_S         : std_logic_vector(6 downto 0) := "0000100";
	constant FUNC_FMUL_S        : std_logic_vector(6 downto 0) := "0001000";
	constant FUNC_FDIV_S        : std_logic_vector(6 downto 0) := "0001100";
	constant FUNC_FSQRT_S       : std_logic_vector(6 downto 0) := "0101100";
	constant FUNC_FSGNJ_S       : std_logic_vector(6 downto 0) := "0010000";
	constant FUNC_FMINMAX_S     : std_logic_vector(6 downto 0) := "0010100";
	constant FUNC_FCONV_F2I_S   : std_logic_vector(6 downto 0) := "1100000";
	constant FUNC_FMV_F2I_S     : std_logic_vector(6 downto 0) := "1110000";
	constant FUNC_FCOMP_S       : std_logic_vector(6 downto 0) := "1010000";
	constant FUNC_FCLASS_S      : std_logic_vector(6 downto 0) := "1110000";
	constant FUNC_FCONV_I2F_S   : std_logic_vector(6 downto 0) := "1101000";
	constant FUNC_FMV_I2F_S     : std_logic_vector(6 downto 0) := "1111000";
	constant FUNC_FIDIV_S       : std_logic_vector(6 downto 0) := "1101100";
	constant FUNC_FISQRT_S      : std_logic_vector(6 downto 0) := "1011010";


	constant FUNC_FMADD         : std_logic_vector(6 downto 0) := "1000011";
	constant FUNC_FMSUB         : std_logic_vector(6 downto 0) := "1000111";
	constant FUNC_FNMSUB        : std_logic_vector(6 downto 0) := "1001011";
	constant FUNC_FNMADD        : std_logic_vector(6 downto 0) := "1001111";


	constant FUNC_FADD_D        : std_logic_vector(6 downto 0) := "0000001";
	constant FUNC_FSUB_D         : std_logic_vector(6 downto 0) := "0000101";
	constant FUNC_FMUL_D        : std_logic_vector(6 downto 0) := "0001001";
	constant FUNC_FDIV_D        : std_logic_vector(6 downto 0) := "0001101";
	constant FUNC_FSQRT_D       : std_logic_vector(6 downto 0) := "0101101";
	constant FUNC_FSGNJ_D       : std_logic_vector(6 downto 0) := "0010001";
	constant FUNC_FMINMAX_D     : std_logic_vector(6 downto 0) := "0010101";
	constant FUNC_FCONV_F2I_D   : std_logic_vector(6 downto 0) := "1100001";
	constant FUNC_FMV_F2I_D     : std_logic_vector(6 downto 0) := "1110001";
	constant FUNC_FCOMP_D       : std_logic_vector(6 downto 0) := "1010001";
	constant FUNC_FCLASS_D      : std_logic_vector(6 downto 0) := "1110001";
	constant FUNC_FCONV_I2F_D   : std_logic_vector(6 downto 0) := "1101001";
	constant FUNC_FMV_I2F_D     : std_logic_vector(6 downto 0) := "1111001";
	constant FUNC_FIDIV_D       : std_logic_vector(6 downto 0) := "1101101";
	constant FUNC_FISQRT_D      : std_logic_vector(6 downto 0) := "1011011";


	constant FUNC_FADD_E        : std_logic_vector(6 downto 0) := "0000010";
	constant FUNC_FSUB_E        : std_logic_vector(6 downto 0) := "0000110";
	constant FUNC_FMUL_E        : std_logic_vector(6 downto 0) := "0001010";
	constant FUNC_FDIV_E        : std_logic_vector(6 downto 0) := "0001110";
	constant FUNC_FSQRT_E       : std_logic_vector(6 downto 0) := "0101110";
	constant FUNC_FSGNJ_E       : std_logic_vector(6 downto 0) := "0010010";
	constant FUNC_FMINMAX_E     : std_logic_vector(6 downto 0) := "0010110";
	constant FUNC_FCONV_F2I_E   : std_logic_vector(6 downto 0) := "1100010";
	constant FUNC_FMV_F2I_E     : std_logic_vector(6 downto 0) := "1110010";
	constant FUNC_FCOMP_E       : std_logic_vector(6 downto 0) := "1010010";
	constant FUNC_FCLASS_E      : std_logic_vector(6 downto 0) := "1110010";
	constant FUNC_FCONV_I2F_E   : std_logic_vector(6 downto 0) := "1101010";
	constant FUNC_FMV_I2F_E     : std_logic_vector(6 downto 0) := "1111010";

	constant FUNC_FCONV_D2S     : std_logic_vector(6 downto 0) := "0100000";
	constant FUNC_FCONV_S2D     : std_logic_vector(6 downto 0) := "0100001";

	constant EXC_INST_ADDR_MISALIGNED    : std_logic_vector(4 downto 0) := "00000";
	constant EXC_INST_ACCESS_FAULT       : std_logic_vector(4 downto 0) := "00001";
	constant EXC_ILLEGAL_INSTRUCTION     : std_logic_vector(4 downto 0) := "00010";
	constant EXC_PRIVILEGED_INSTRUCTION  : std_logic_vector(4 downto 0) := "00011";
	constant EXC_FLOATING_POINT_DISABLED : std_logic_vector(4 downto 0) := "00100";
	-- 5: unused
	constant EXC_SYSTEM_CALL             : std_logic_vector(4 downto 0) := "00110";
	constant EXC_BREAKPOINT              : std_logic_vector(4 downto 0) := "00111";
	constant EXC_LOAD_ADDR_MISALIGNED    : std_logic_vector(4 downto 0) := "01000";
	constant EXC_STORE_ADDR_MISALIGNED   : std_logic_vector(4 downto 0) := "01001";
	constant EXC_LOAD_ACCESS_FAULT       : std_logic_vector(4 downto 0) := "01010";
	constant EXC_STORE_ACCESS_FAULT      : std_logic_vector(4 downto 0) := "01011";
	-- 12 - 15: unused
	constant EXC_EXT_INTERRUPT_0         : std_logic_vector(4 downto 0) := "10000";
	constant EXC_EXT_INTERRUPT_1         : std_logic_vector(4 downto 0) := "10001";
	constant EXC_EXT_INTERRUPT_2         : std_logic_vector(4 downto 0) := "10010";
	constant EXC_EXT_INTERRUPT_3         : std_logic_vector(4 downto 0) := "10011";
	constant EXC_EXT_INTERRUPT_4         : std_logic_vector(4 downto 0) := "10100";
	constant EXC_EXT_INTERRUPT_5         : std_logic_vector(4 downto 0) := "10101";
	constant EXC_EXT_INTERRUPT_6         : std_logic_vector(4 downto 0) := "10110";
	constant TIMER_INTERRUPT             : std_logic_vector(4 downto 0) := "10111";
	-- 24 - 31: unused


	constant CSR_SCALL   : std_logic_vector(11 downto 0) := x"000";
	constant CSR_SBREAK  : std_logic_vector(11 downto 0) := x"001";
	constant CSR_EPC     : std_logic_vector(11 downto 0) := x"502";
	constant CSR_TIMER   : std_logic_vector(11 downto 0) := x"506";
	constant CSR_COMPARE : std_logic_vector(11 downto 0) := x"507";
	constant CSR_EVEC    : std_logic_vector(11 downto 0) := x"508";
	constant CSR_CAUSE   : std_logic_vector(11 downto 0) := x"509";
	constant CSR_STATUS  : std_logic_vector(11 downto 0) := x"50A";
	constant CSR_HARTID  : std_logic_vector(11 downto 0) := x"50B";

	-- custom FGPM CSRs (all read only)
	constant CSR_FGMP_MAXCID  : std_logic_vector(11 downto 0) := x"C70";
	constant CSR_FGMP_CID     : std_logic_vector(11 downto 0) := x"C71";
	constant CSR_FGMP_NOCDIM  : std_logic_vector(11 downto 0) := x"C72";

	constant CSR_SRET : std_logic_vector(11 downto 0) := x"800";

	constant CSR_CYCLE    : std_logic_vector(11 downto 0) := x"C00";
	constant CSR_CYCLEH   : std_logic_vector(11 downto 0) := x"C80";
	constant CSR_TIME     : std_logic_vector(11 downto 0) := x"C01";
	constant CSR_TIMEH    : std_logic_vector(11 downto 0) := x"C81";
	constant CSR_INSTRET  : std_logic_vector(11 downto 0) := x"C02";
	constant CSR_INSTRETH : std_logic_vector(11 downto 0) := x"C82";

	constant FCSR_FFLAGS    : std_logic_vector(11 downto 0) := x"001";
	constant FCSR_FRM       : std_logic_vector(11 downto 0) := x"002";
	constant FCSR_FCSR      : std_logic_vector(11 downto 0) := x"003";

	constant FUNC_FGMP_SEND  : std_logic_vector(2 downto 0) := "000";
	constant FUNC_FGMP_CONG  : std_logic_vector(2 downto 0) := "001";
	constant FUNC_FGMP_RECV  : std_logic_vector(2 downto 0) := "100";
	constant FUNC_FGMP_PROBE : std_logic_vector(2 downto 0) := "101";
	constant FUNC_FGMP_WAIT  : std_logic_vector(2 downto 0) := "110";
	constant FUNC_FGMP_ANY   : std_logic_vector(2 downto 0) := "111";

end package libeu;
