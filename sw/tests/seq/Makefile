include ../../all_single_c_files.mk

$(BUILD_DIR)argv.$(ARCH).elf: argv.c
	$(CC) $(CFLAGS) -Wall -O2 -g -o $@ $^ $(ADD_CFLAGS) -lplastic $(LDFLAGS)
