RCMC_ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))../../../))
include $(RCMC_ROOT)config_default.mk

INCLUDE=include

C_FILES = $(wildcard src/*.c)
PROGRAMS = $(C_FILES:src/%.c=%)
PTHREAD_FILES = $(PROGRAMS:%=build/%.x86)
MPI_FILES = $(PROGRAMS:%=build/%.mpi.x86)



all: pthread mpi rcmc

pthread: build/ $(PTHREAD_FILES)

mpi: build/ $(MPI_FILES)

fgmp.riscv: build/ $(PROGRAMS:%=build/%.fgmp.riscv)

fgmp.rv64i: build/ $(PROGRAMS:%=build/%.fgmp.rv64i)


build/%.x86: src/%.c
	clang -I$(INCLUDE) -o $@ -pthread $< -lm

build/%.mpi.x86: src/%.c
	mpicc -I$(INCLUDE) -o $@ -DPAROP_IMPL_MPI $< -lm

build/%.fgmp.riscv: src/%.c
	$(CC_RV64IMFD) -march=rv64imfd -I$(INCLUDE) -o $@ -DPAROP_IMPL_MPI \
	  -I$(RCMC_ROOT)sw/lib/arch/riscv/include -I$(RCMC_ROOT)sw/lib/include $< \
	  -T$(RCMC_ROOT)sw/lib/arch/riscv/minimal.ld -L$(RCMC_ROOT)sw/lib/arch/riscv/lib \
	  -nostdlib -lmpi -lc -lm -lgcc -lplastic

build/%.fgmp.rv64i: src/%.c
	$(CC_RV64I) -march=rv64i -I$(INCLUDE) -o $@ -DPAROP_IMPL_MPI \
	  -I$(RCMC_ROOT)sw/lib/arch/rv64i/include -I$(RCMC_ROOT)sw/lib/include $< \
	  -T$(RCMC_ROOT)sw/lib/arch/rv64i/minimal.ld -L$(RCMC_ROOT)sw/lib/arch/rv64i/lib \
	  -nostdlib -lmpi -lc -lm -lgcc -lplastic




.PHONY: clean
clean:
	-rm -f build/*.x86 build/*.riscv build/*.rv64i

build/:
	-mkdir build

