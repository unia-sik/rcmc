ARCH=riscv
include ../../arch_dependent.mk

LIB_COPPER = ../lib/libcopper.a
LIB_PLASTIC = ../lib/libplastic.a
BUILD_DIR = build/

C_FILES = $(wildcard *.c)
O_FILES = $(C_FILES:%.c=$(BUILD_DIR)%.o)


.PHONY: all
all: $(BUILD_DIR) $(LIB_COPPER) $(LIB_PLASTIC)

.PHONY: clean
clean:
	-rm -rf $(BUILD_DIR)
	-rm $(LIB_COPPER) $(LIB_PLASTIC)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)%.o: %.c
	$(CC) $(CFLAGS) -g -O2 -Wall -c -o $@ $^

$(BUILD_DIR)%.copper.o: %.S
	$(CC) $(CFLAGS) -DBAREMETAL -g -O2 -Wall -c -o $@ $^

$(BUILD_DIR)%.plastic.o: %.S
	$(CC) $(CFLAGS) -g -O2 -Wall -c -o $@ $^

$(LIB_COPPER): $(O_FILES) $(BUILD_DIR)crt.copper.o
	ar -rc $@ $^

$(LIB_PLASTIC): $(O_FILES) $(BUILD_DIR)crt.plastic.o
	ar -rc $@ $^

