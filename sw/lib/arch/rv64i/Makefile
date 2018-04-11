# build all libraries for this target

.PHONY: all
all:
	$(MAKE) -C src
	ARCH=rv64i $(MAKE) -C ../../mpi

.PHONY: clean
clean:
	$(MAKE) -C src clean
	ARCH=rv64i $(MAKE) -C ../../mpi clean
