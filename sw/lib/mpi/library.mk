# Build all .c files and put the .o files in a library in the arch tree
include ../../arch/arch_dependent.mk

BUILD_DIR = $(RCMC_ROOT)sw/lib/mpi/build/
C_FILES = $(wildcard *.c)
O_FILES = $(C_FILES:%.c=$(BUILD_DIR)%.$(ARCH).o)

.PHONY: all
all: $(BUILD_DIR) $(O_FILES)

.PHONY: clean
clean:
	rm -f $(LIB_PATH)
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)%.$(ARCH).o: %.c
	$(CC) $(CFLAGS) $(ADD_CFLAGS) -s -O2 -Wall -c -o $@ $^
