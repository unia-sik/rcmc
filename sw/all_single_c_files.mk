# Makefile to build executables from all single .c files in the current directory

include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))lib/arch/arch_dependent.mk

BUILD_DIR = build/

C_FILES = $(wildcard *.c)
PROGRAMS = $(C_FILES:%.c=%)
ELF_FILES = $(PROGRAMS:%=$(BUILD_DIR)%.$(ARCH).elf)



$(BUILD_DIR)%.$(ARCH).elf: %.c
	$(CC) $(CFLAGS) -Wall -O2 -g -o $@ $^ $(ADD_CFLAGS) $(LDFLAGS)


all: libraries $(BUILD_DIR) $(ELF_FILES)

clean:
	-rm -rf $(BUILD_DIR)

# build arch specific libraries
libraries:
	make -C $(RCMC_ROOT)sw/lib/arch/$(ARCH)

$(BUILD_DIR):
	-mkdir $(BUILD_DIR)

