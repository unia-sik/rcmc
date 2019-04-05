# Set architecture dependent variables
# Input: $(ARCH) architecture name

ifeq ($(ARCH),)
  $(error ARCH= not specified)
endif

RCMC_ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))../../../../))
include $(RCMC_ROOT)config_default.mk


ARCH_PATH=$(RCMC_ROOT)sw/lib/arch/$(ARCH)
LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi_be -lc -lm -lgcc -lcopper
#LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi_pnoo -lc -lm -lgcc -lcopper

ifeq ($(ARCH),armv3)
  CC=$(CC_ARM)
  CFLAGS=-march=armv5 -msoft-float -DARMV6M
#  ARCH_PATH=$(RCMC_ROOT)sw/lib/arch/armv3
#  LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi -lc -lm -lgcc -lcopper
else
ifeq ($(ARCH),armv6m)
    CC=$(CC_ARM)
    CFLAGS=-mcpu=cortex-m0 -mthumb -DARMV6M
#    ARCH_PATH=$(RCMC_ROOT)sw/lib/arch/armv6m
#    LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi -lc -lm -lgcc -lcopper
else
ifeq ($(ARCH),riscv)
      CC=$(CC_RV64IMFD)
      CFLAGS=-march=rv64imfd -mno-fdiv
#      CFLAGS=-march=rv64imfd
#      ARCH_PATH=$(RCMC_ROOT)sw/lib/arch/riscv
#      LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi -lc -lm -lgcc -lcopper
else
ifeq ($(ARCH),rv64i)
        CC=$(CC_RV64I)
        CFLAGS=-march=rv64i
#        ARCH_PATH=$(RCMC_ROOT)sw/lib/arch/rv64i
#        LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi -lc -lm -lgcc -lcopper
else
ifeq ($(ARCH),rvmpb)
          CC=riscv64-unknown-elf-gcc
#          ARCH_PATH=$(RCMC_ROOT)sw/lib/arch/rvmpb
#          LDFLAGS = -L$(ARCH_PATH)/lib -nostdlib -lmpi -lc -lm -lgcc -lcopper
else
            $(error Unknown architecture $(ARCH))
endif
endif
endif
endif
endif

CFLAGS += -T$(ARCH_PATH)/minimal.ld -I$(ARCH_PATH)/include \
    -I$(RCMC_ROOT)sw/lib/include
#LDFLAGS = -L$(RCMC_ROOT)baremetal/arch/$(ARCH)/lib \
#    -nostdlib -lsilver -lc -lm -lgcc -lcopper
