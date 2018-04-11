library IEEE;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.libeu.all;



entity FPU is
    Port (
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
end FPU;

architecture Behavioral of FPU is



Signal sig_signj_P : std_logic_vector(63 downto 0);
Signal sig_signj_Ex : std_logic_vector(4 downto 0);

Signal sig_class_P : std_logic_vector(63 downto 0);
Signal sig_class_Ex : std_logic_vector(4 downto 0);

Signal sig_minmax_P : std_logic_vector(63 downto 0);
Signal sig_minmax_Ex : std_logic_vector(4 downto 0);

Signal sig_comp_P : std_logic_vector(63 downto 0);
Signal sig_comp_Ex : std_logic_vector(4 downto 0);

Signal sig_i2f_P : std_logic_vector(63 downto 0);
Signal sig_i2f_Ex : std_logic_vector(4 downto 0);

Signal sig_f2i_P : std_logic_vector(63 downto 0);
Signal sig_f2i_Ex : std_logic_vector(4 downto 0);

Signal sig_f2f_P : std_logic_vector(63 downto 0);
Signal sig_f2f_Ex : std_logic_vector(4 downto 0);

Signal sig_fma_P : std_logic_vector(63 downto 0);
Signal sig_fma_Ex : std_logic_vector(4 downto 0);

Signal sig_fdiv_P : std_logic_vector(63 downto 0);
Signal sig_fdiv_Ex : std_logic_vector(4 downto 0);
Signal sig_fdiv_M1 : std_logic_vector(63 downto 0);
Signal sig_fdiv_M2 : std_logic_vector(63 downto 0);

Signal sig_fsqrt_P : std_logic_vector(63 downto 0);
Signal sig_fsqrt_Ex : std_logic_vector(4 downto 0);
Signal sig_fsqrt_M1 : std_logic_vector(63 downto 0);
Signal sig_fsqrt_M2 : std_logic_vector(63 downto 0);

Signal sig_f2i_Ready : std_logic;
Signal sig_i2f_Ready : std_logic;
Signal sig_fma_Ready : std_logic;

begin

    FPUCLASS_i: FPUCLASS port map(A,D,FMT,Funct,RMI,AuxI,sig_class_Ex,sig_class_P);
    FPUSIGNJ_i: FPUSIGNJ port map(A,B,Funct,RMI,sig_signj_Ex,sig_signj_P);
    FPUMINMAX_i: FPUMINMAX port map(A,B,Funct,RMI,sig_minmax_Ex,sig_minmax_P);
    FPUCOMP_i: FPUCOMP port map(A,B,Funct,RMI,sig_comp_Ex,sig_comp_P);
    FPUtoINT_i: FPUtoINT port map(A,FMT,Funct,RMI,FPUOPI,nRST,CLK,sig_f2i_Ex,sig_f2i_P,sig_f2i_Ready);
    INTtoFPU_i: INTtoFPU port map(D,FMT,Funct,RMI,FPUOPI,nRST,CLK,sig_i2f_Ex,sig_i2f_P,sig_i2f_Ready);
    FPUtoFPU_i: FPUtoFPU port map(A,FMT,Funct,RMI,sig_f2f_Ex,sig_f2f_P);
    FPUFMA_i: FPUFMA port map(A,B,C,FMT,Funct,RMI,FPUOPI,nRST,CLK,sig_fma_Ex,sig_fma_P,sig_fma_Ready);
    FPUDIV_i : FPUDIV port map(A,B,FMT,Funct,RMI,sig_fdiv_Ex,sig_fdiv_P,sig_fdiv_M1,sig_fdiv_M2);
    FPUSQRT_i : FPUSQRT port map(A,FMT,Funct,RMI,sig_fsqrt_Ex,sig_fsqrt_P,sig_fsqrt_M1,sig_fsqrt_M2);

	process (Funct,FPUOPI,FPUCyclesI,sig_i2f_P,sig_i2f_Ex,sig_f2i_P,sig_f2i_Ex,sig_f2f_P,sig_f2f_Ex
					,sig_class_P,sig_class_Ex,sig_signj_P,sig_signj_Ex,sig_minmax_P,sig_minmax_Ex,sig_comp_P,sig_comp_Ex
					,sig_fdiv_Ex,sig_fdiv_P,sig_fdiv_M1,sig_fdiv_M2,sig_fsqrt_Ex,sig_fsqrt_P,sig_fsqrt_M1,sig_fsqrt_M2
					,sig_f2i_Ready,sig_i2f_Ready,sig_fma_Ex,sig_fma_P,sig_fma_Ready)

	variable varP  : std_logic_vector(63 downto 0);
	variable varM1 : std_logic_vector(63 downto 0);
	variable varM2 : std_logic_vector(63 downto 0);
	variable varEx : std_logic_vector(4 downto 0);
	variable varReady : std_logic;



	begin

		varP  := (others => '0');
		varM1 := (others => '0');
		varM2 := (others => '0');
		varEx := (others => '0');
		varReady := '0';



		if sig_f2i_Ready = '1' then

			varP := sig_f2i_P;
			varM1 := varP;
			varEx := sig_f2i_Ex;
			varReady := sig_f2i_Ready;

		elsif sig_i2f_Ready = '1' then

			varP := sig_i2f_P;
			varM1 := varP;
			varEx := sig_i2f_Ex;
			varReady := sig_i2f_Ready;

		elsif sig_fma_Ready = '1' then

			varP := sig_fma_P;
			varM1 := varP;
			varEx := sig_fma_Ex;
			varReady := sig_fma_Ready;

		elsif FPUCyclesI = '0' then

			if FPUOPI = '1' then

				if Funct = FUNC_FCLASS_S then

					varP := sig_class_P;
					varM1 := varP;
					varEx := sig_class_Ex;
					varReady := '0';

				elsif Funct = FUNC_FCLASS_D then

					varP := sig_class_P;
					varM1 := varP;
					varEx := sig_class_Ex;
					varReady := '0';

				elsif Funct = FUNC_FMV_I2F_S then

					varP := sig_class_P;
					varM1 := varP;
					varEx := sig_class_Ex;
					varReady := '0';

				elsif Funct = FUNC_FMV_I2F_D then

					varP := sig_class_P;
					varM1 := varP;
					varEx := sig_class_Ex;
					varReady := '0';

				elsif Funct = FUNC_FSGNJ_S then

					varP := sig_signj_P;
					varM1 := varP;
					varEx := sig_signj_Ex;
					varReady := '0';

				elsif Funct = FUNC_FSGNJ_D then

					varP := sig_signj_P;
					varM1 := varP;
					varEx := sig_signj_Ex;
					varReady := '0';

				elsif Funct = FUNC_FMINMAX_S then

					varP := sig_minmax_P;
					varM1 := varP;
					varEx := sig_minmax_Ex;
					varReady := '0';
				elsif Funct = FUNC_FMINMAX_D then

					varP := sig_minmax_P;
					varM1 := varP;
					varEx := sig_minmax_Ex;
					varReady := '0';

				elsif Funct = FUNC_FCOMP_S then

					varP := sig_comp_P;
					varM1 := varP;
					varEx := sig_comp_Ex;
					varReady := '0';

				elsif Funct = FUNC_FCOMP_D then

					varP := sig_comp_P;
					varM1 := varP;
					varEx := sig_comp_Ex;
					varReady := '0';

				elsif Funct = FUNC_FCONV_D2S then

					varP := sig_f2f_P;
					varM1 := varP;
					varEx := sig_f2f_Ex;
					varReady := '0';

				elsif Funct = FUNC_FCONV_S2D then

					varP := sig_f2f_P;
					varM1 := varP;
					varEx := sig_f2f_Ex;
					varReady := '0';

				elsif Funct = FUNC_FDIV_S then

					varP := sig_fdiv_P;
					varM1 := sig_fdiv_M1;
					varM2 := sig_fdiv_M2;
					varEx := sig_fdiv_Ex;
					varReady := '0';

				elsif Funct = FUNC_FDIV_D then

					varP := sig_fdiv_P;
					varM1 := sig_fdiv_M1;
					varM2 := sig_fdiv_M2;
					varEx := sig_fdiv_Ex;
					varReady := '0';

				elsif Funct = FUNC_FSQRT_S then

					varP := sig_fsqrt_P;
					varM1 := sig_fsqrt_M1;
					varM2 := sig_fsqrt_M2;
					varEx := sig_fsqrt_Ex;
					varReady := '0';

				elsif Funct = FUNC_FSQRT_D then

					varP := sig_fsqrt_P;
					varM1 := sig_fsqrt_M1;
					varM2 := sig_fsqrt_M2;
					varEx := sig_fsqrt_Ex;
					varReady := '0';

				end if;

			end if;

		end if;


		P <= varP;
		M1 <= varM1;
		M2 <= varM2;
		Ex <= varEx;
		Ready <= varReady;
	end process;

end Behavioral;
