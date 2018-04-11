include ../arch/arch_dependent.mk

LIB_NAME = ../arch/$(ARCH)/lib/libmpi.a

BUILD_DIR = build/
C_FILES = $(wildcard *.c)
O_FILES = $(C_FILES:%.c=$(BUILD_DIR)%.$(ARCH).o)

.PHONY: all
all: $(BUILD_DIR) $(LIB_NAME)

.PHONY: clean
clean:
	rm -f $(LIB_NAME)
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)%.$(ARCH).o: %.c
	$(CC) $(CFLAGS) -g -O2 -Wall -c -o $@ $^

$(LIB_NAME): $(O_FILES)
	ar -rc $@ $^
