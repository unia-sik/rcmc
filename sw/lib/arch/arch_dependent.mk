# Set architecture dependent variables
# Input: $(ARCH) architecture name

ifeq ($(ARCH),)
  $(error ARCH= not specified)
endif

#$(info MPILIB=$(MPILIB))
#ifeq ($(MPILIB),)
#  $(error MPILIB= not specified)
#endif

RCMC_ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))../../../../))
include $(RCMC_ROOT)config_default.mk


ARCH_PATH=$(RCMC_ROOT)sw/lib/arch/$(ARCH)
#LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi_$(MPILIB) -lc -lm -lgcc -lcopper
LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi -lc -lm -lgcc -lcopper

ifeq ($(ARCH),armv3)
  CC=$(CC_ARM)
  CFLAGS=-march=armv5 -msoft-float -DARMV6M
else
ifeq ($(ARCH),armv6m)
    CC=$(CC_ARM)
    CFLAGS=-mcpu=cortex-m0 -mthumb -DARMV6M
else
ifeq ($(ARCH),riscv)
      CC=$(CC_RV64IMAFD)
      CFLAGS=-march=rv64imafd -mno-fdiv
else
ifeq ($(ARCH),rv64i)
        CC=$(CC_RV64I)
        CFLAGS=-march=rv64i
else
ifeq ($(ARCH),rvmpb)
          CC=riscv64-unknown-elf-gcc
else
            $(error Unknown architecture $(ARCH))
endif
endif
endif
endif
endif

#CFLAGS += -DRCMC_MPILIB_$(MPILIB) -T$(ARCH_PATH)/minimal.ld \
#    -I$(ARCH_PATH)/include -I$(RCMC_ROOT)sw/lib/include

CFLAGS += -T$(ARCH_PATH)/minimal.ld \
    -I$(ARCH_PATH)/include -I$(RCMC_ROOT)sw/lib/include
