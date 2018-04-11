ARCH=armv6m
include ../../arch_dependent.mk

LIB_NAME = ../lib/libcopper.a

BUILD_DIR = build/
C_FILES = $(wildcard *.c)
S_FILES = $(wildcard *.S)

#ifneq ($(C_FILES),)
  O_FILES += $(C_FILES:%.c=$(BUILD_DIR)%.o)
#endif
#ifneq ($(S_FILES),)
  O_FILES += $(S_FILES:%.S=$(BUILD_DIR)%.o)
#endif

.PHONY: all
all: $(BUILD_DIR) $(LIB_NAME)

.PHONY: clean
clean:
	rm -f $(LIB_NAME)
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)%.o: %.c
	$(CC) $(CFLAGS) -g -O2 -Wall -c -o $@ $^

$(BUILD_DIR)%.o: %.S
	$(CC) $(CFLAGS) -g -O2 -Wall -c -o $@ $^

$(LIB_NAME): $(O_FILES)
	ar -rc $@ $^

