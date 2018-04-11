# build all libraries for this target

.PHONY: all
all:
	$(MAKE) -C src
	ARCH=riscv $(MAKE) -C ../../mpi

.PHONY: clean
clean:
	$(MAKE) -C src clean
	ARCH=riscv $(MAKE) -C ../../mpi clean
