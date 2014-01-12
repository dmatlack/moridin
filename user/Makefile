#
#
# User Makefile
#
#

# location of source code for user programs
PROGS := progs
# location of executables for user programs
BIN := bin

AUTOSTART := $(shell mkdir -p $(BIN))

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

CFLAGS := -g -std=c99 -ffreestanding $(WARNINGS)

# TODO: add a C library for user programs to use
INCLUDES := 

.PHONY: default clean

default: all

clean:
	rm -f *.o
	rm -f progs/*.o
	rm -f bin/*

###############################################################################
# User Programs
###############################################################################

all: spin

spin: crt0.o progs/spin.o
	$(LD) -T user.ld $? -o $(BIN)/spin

###############################################################################
# General Make fulrs for filetypes
###############################################################################
%.o: %.S
	$(CC) $(CFLAGS) -DASSEMBLER $(INCLUDES) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
