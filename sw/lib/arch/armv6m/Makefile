# build all libraries for this target

.PHONY: all
all:
	$(MAKE) -C src
	ARCH=armv6m $(MAKE) -C ../../mpi

.PHONY: clean
clean:
	$(MAKE) -C src clean
	ARCH=armv6m $(MAKE) -C ../../mpi clean
