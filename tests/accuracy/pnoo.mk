# Build executables for the cycle-by-cycle comparison of MacSim and the FPGA.
# They are special, because the FGPA has many restrictions (RV64I, small memory, slow, ...)
#
# Architecture: rv64i, one-to-one with implicit flow control


ARCH=rv64i
#ADD_CFLAGS=-DRCMC_MPI_FLOWCONTROL
#ADD_LDFLAGS=-lmpi

include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))../../sw/lib/arch/arch_dependent.mk
# does set $(RCMC_ROOT)

DIR_BUILD = build/

DIR_RISCVTESTS = $(RCMC_ROOT)sw/tests/riscv-tests/
DIR_SEQ = $(RCMC_ROOT)sw/tests/seq/
DIR_4X4 = $(RCMC_ROOT)sw/tests/special/4x4/
DIR_MPI = $(RCMC_ROOT)sw/tests/mpi/
DIR_FGMP = $(RCMC_ROOT)sw/tests/fgmp/
DIR_PNOO = $(RCMC_ROOT)sw/tests/special/pnoo/





S_SEQ = $(notdir $(wildcard $(DIR_SEQ)*.c))
E_SEQ = $(S_SEQ:%.c=$(DIR_BUILD)seq_%.$(ARCH).elf)

S_PNOO = $(notdir $(wildcard $(DIR_PNOO)*.c))
E_PNOO = $(S_PNOO:%.c=$(DIR_BUILD)pnoo_%.$(ARCH).elf)

S_MPI = $(notdir $(wildcard $(DIR_MPI)*.c))
E_MPI = $(S_MPI:%.c=$(DIR_BUILD)mpi_%.$(ARCH).elf)

S_FGMP = $(notdir $(wildcard $(DIR_FGMP)*.c))
E_FGMP = $(S_FGMP:%.c=$(DIR_BUILD)fgmp_%.$(ARCH).elf)

S_BENCH = bitonic_sort cg ocean
E_BENCH = $(S_BENCH:%=$(DIR_BUILD)bench_%.$(ARCH).elf)



all: libraries $(DIR_BUILD) $(E_SEQ) $(E_PNOO) $(E_MPI) $(E_FGMP) $(E_BENCH)


$(DIR_BUILD)seq_%.$(ARCH).elf: $(DIR_SEQ)%.c
	$(CC) -DRCMC_MPI_FLOWCONTROL $(CFLAGS) -O2 -o $@ $^ $(LDFLAGS)

$(DIR_BUILD)pnoo_%.$(ARCH).elf: $(DIR_PNOO)%.c
	$(CC) -DRCMC_MPI_FLOWCONTROL $(CFLAGS) -O2 -o $@ $^ $(LDFLAGS)

$(DIR_BUILD)mpi_%.$(ARCH).elf: $(DIR_MPI)%.c
	$(CC) -DRCMC_MPI_FLOWCONTROL $(CFLAGS) -O2 -o $@ $^ $(LDFLAGS)

$(DIR_BUILD)fgmp_%.$(ARCH).elf: $(DIR_FGMP)%.c
	$(CC) -DRCMC_MPI_FLOWCONTROL $(CFLAGS) -O2 -o $@ $^ $(LDFLAGS)

# -------

$(DIR_BUILD)bench_ocean.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/ocean.c
	$(CC) -DRCMC_MPI_FLOWCONTROL $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_LOG2_N=3 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)

$(DIR_BUILD)bench_cg.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/cg.c
	$(CC) -DRCMC_MPI_FLOWCONTROL $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_CLASS=0 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)

$(DIR_BUILD)bench_bitonic_sort.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/bitonic_sort.c
	$(CC) -DRCMC_MPI_FLOWCONTROL $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_PROBLEM_SIZE=0 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)




clean:
	-rm -rf $(DIR_BUILD)

# build arch specific libraries
#
# FIXME: don't know why MPILIB=$(MPILIB) is necessary
libraries:
	make -C $(RCMC_ROOT)sw/lib/arch/$(ARCH)

$(DIR_BUILD):
	-mkdir $(DIR_BUILD)

