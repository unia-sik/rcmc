# build all libraries for this target

.PHONY: all
all:
	$(MAKE) -C src
	ARCH=armv3 $(MAKE) -C ../../mpi

.PHONY: clean
clean:
	$(MAKE) -C src clean
	ARCH=armv3 $(MAKE) -C ../../mpi clean
