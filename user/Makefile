#
#
# User Makefile
#
#

# default architecture to build for
ARCH=ARCH_X86

# location of source code for user programs
PROGS := progs

# location of executables for user programs
BIN := bin

AUTOSTART := $(shell mkdir -p $(BIN))

LIB_CFILES =  			$(shell find $(shell pwd)/lib -type f -name "*.c")
LIB_SFILES = crt0.S $(shell find $(shell pwd)/lib -type f -name "*.S")
LIB_OFILES = $(patsubst %.c,%.o,$(LIB_CFILES)) $(patsubst %.S,%.o,$(LIB_SFILES))

###############################################################################
# Build Toolchain
###############################################################################
CC = i586-elf-gcc
LD = i586-elf-ld

WARNINGS := \
			-Wall \
			-Wextra \
			-Wshadow \
			-Wcast-align \
			-Wwrite-strings \
			-Wredundant-decls \
			-Wnested-externs \
			-Winline \
			-Wuninitialized \
			-Werror \

CFLAGS := -g -std=c99 -ffreestanding -D$(ARCH) $(WARNINGS)

# TODO: add a C library for user programs to use
INCLUDES := 

.PHONY: default clean

default: all

clean:
	rm -f *.o
	rm -f progs/*.o
	rm -f lib/*.o
	rm -f bin/*

###############################################################################
# User Programs
###############################################################################
all: init


init: $(LIB_OFILES) progs/init.o
	$(LD) -T user.ld $? -o $(BIN)/init


###############################################################################
# General Make fulrs for filetypes
###############################################################################
%.o: %.S
	$(CC) $(CFLAGS) -DASSEMBLER $(INCLUDES) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
