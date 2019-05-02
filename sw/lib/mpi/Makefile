# build all MPI libraries for a specific architecture

include ../arch/arch_dependent.mk

LIB_PATH = $(RCMC_ROOT)sw/lib/arch/$(ARCH)/lib/libmpi.a



.PHONY: all
all:
	$(MAKE) -C hs
	$(MAKE) -C fc
	ar -rc $(LIB_PATH) build/*.o


.PHONY: clean
clean:
	$(MAKE) -C hs clean
	$(MAKE) -C fc clean
	-rm $(LIB_PATH)
