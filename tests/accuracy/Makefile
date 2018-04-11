# Build executables for the cycle-by-cycle comparison of MacSim and the FPGA.
# They are special, because the FGPA has many restrictions (RV64I, small memory, slow, ...)

include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))../../sw/lib/arch/arch_dependent.mk
# does set $(RCMC_ROOT)

DIR_BUILD = build/

DIR_RISCVTESTS = $(RCMC_ROOT)sw/tests/riscv-tests/
DIR_SEQ = $(RCMC_ROOT)sw/tests/seq/
DIR_MPI = $(RCMC_ROOT)sw/tests/mpi/
DIR_FGMP = $(RCMC_ROOT)sw/tests/fgmp/
DIR_4X4 = $(RCMC_ROOT)sw/tests/noc/





S_RV64UI = $(notdir $(wildcard $(DIR_RISCVTESTS)/rv64ui/*.S))
E_RV64UI = $(S_RV64UI:%.S=$(DIR_BUILD)rv64ui_%.$(ARCH).elf)

S_SEQ = $(notdir $(wildcard $(DIR_SEQ)*.c))
E_SEQ = $(S_SEQ:%.c=$(DIR_BUILD)seq_%.$(ARCH).elf)

S_4X4 = $(notdir $(wildcard $(DIR_4X4)4x4_*.c))
E_4X4 = $(S_4X4:%.c=$(DIR_BUILD)4x4_%.$(ARCH).elf)

S_MPI = $(notdir $(wildcard $(DIR_MPI)*.c))
E_MPI = $(S_MPI:%.c=$(DIR_BUILD)mpi_%.$(ARCH).elf)

S_FGMP = $(notdir $(wildcard $(DIR_FGMP)*.c))
E_FGMP = $(S_FGMP:%.c=$(DIR_BUILD)fgmp_%.$(ARCH).elf)

S_BENCH = bitonic_sort cg ocean
E_BENCH = $(S_BENCH:%=$(DIR_BUILD)bench_%.$(ARCH).elf)




all: $(DIR_BUILD) $(E_RV64UI) $(E_SEQ) $(E_MPI) $(E_FGMP) $(E_4X4) $(E_BENCH)

$(DIR_BUILD)rv64ui_%.$(ARCH).elf: $(DIR_RISCVTESTS)rv64ui/%.S
	$(CC) -T$(DIR_RISCVTESTS)link.ld -nostdlib -o $@ $^ -I$(DIR_RISCVTESTS) -I$(DIR_RISCVTESTS)macros/scalar

$(DIR_BUILD)seq_%.$(ARCH).elf: $(DIR_SEQ)%.c
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

$(DIR_BUILD)4x4_%.$(ARCH).elf: $(DIR_4X4)%.c
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

$(DIR_BUILD)mpi_%.$(ARCH).elf: $(DIR_MPI)%.c
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

$(DIR_BUILD)fgmp_%.$(ARCH).elf: $(DIR_FGMP)%.c
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

# -------

$(DIR_BUILD)bench_ocean.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/ocean.c
	$(CC) $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_LOG2_N=3 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)

$(DIR_BUILD)bench_cg.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/cg.c
	$(CC) $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_CLASS=0 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)

$(DIR_BUILD)bench_bitonic_sort.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/bitonic_sort.c
	$(CC) $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_PROBLEM_SIZE=0 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)












clean:
	-rm -rf $(DIR_BUILD)

$(DIR_BUILD):
	-mkdir $(DIR_BUILD)

