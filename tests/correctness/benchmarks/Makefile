include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))../../../sw/lib/arch/arch_dependent.mk

PROGRAMS = ocean cg bitonic_sort
ELF_FILES = $(PROGRAMS:%=%.$(ARCH).elf)

all: $(ELF_FILES)

ocean.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/ocean.c
	$(CC) $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_LOG2_N=5 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)
#	cp $(RCMC_ROOT)sw/benchmarks/s/ocean/correct.5 ocean.correct

cg.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/cg.c
	$(CC) $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_CLASS=0 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)

bitonic_sort.$(ARCH).elf: $(RCMC_ROOT)sw/benchmarks/src/bitonic_sort.c
	$(CC) $(CFLAGS) -O2 -g -o $@ $^ -DPAROP_IMPL_MPI -DDEFAULT_PROBLEM_SIZE=0 \
	  -I$(RCMC_ROOT)sw/benchmarks/include $(LDFLAGS)

clean:
	-rm -f *.elf *.correct


