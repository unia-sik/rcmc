# Build all .c files and put the .o files in a library in the arch tree
include ../../arch/arch_dependent.mk

LIB_PATH = $(RCMC_ROOT)sw/lib/arch/$(ARCH)/lib/$(LIB_NAME).a

BUILD_DIR = build/
C_FILES = $(wildcard *.c)
O_FILES = $(C_FILES:%.c=$(BUILD_DIR)%.$(ARCH).o)

.PHONY: all
all: $(BUILD_DIR) $(LIB_PATH)

.PHONY: clean
clean:
	rm -f $(LIB_PATH)
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)%.$(ARCH).o: %.c
	$(CC) $(CFLAGS) -g -O2 -Wall -c -o $@ $^

$(LIB_PATH): $(O_FILES)
	ar -rc $@ $^
