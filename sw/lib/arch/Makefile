# build all libraries of all target architectures

.PHONY: all
all:
	$(MAKE) -C armv3
	$(MAKE) -C armv6m
	$(MAKE) -C riscv
	$(MAKE) -C rv64i

.PHONY: clean
clean:
	$(MAKE) -C armv3 clean
	$(MAKE) -C armv6m clean
	$(MAKE) -C riscv clean
	$(MAKE) -C rv64i clean
