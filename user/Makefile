###############################################################################
#
# User Makefile
#
###############################################################################
include ../config.mk

BIN = bin

INCLUDES = -Iinclude

.PHONY: all sys clean

all: sys init

clean:
	rm -rf $(SYS_OFILES)
	rm -rf $(BIN)/*
	rm -rf progs/*.o

#
# Sys is a binary that is linked statically into all user executables. It
# contains the code needed to make system calls, initializes the runtime,
# etc.
#
SYS_CFILES = $(shell find sys/ -type f -name "*.c")
SYS_SFILES = $(shell find sys/ -type f -name "*.S")
SYS_OFILES = $(patsubst %.c,%.o,$(SYS_CFILES)) $(patsubst %.S,%.o,$(SYS_SFILES))

sys: $(SYS_OFILES)

#
# PUT USER PROGRAMS BELOW THIS POINT
#
# 	- All user programs (targets) should include sys as a dependency
# 	- List all source files needed by the program as .o files. (e.g. if
# 		you need myprogram.c, use myprogram.o)
#

init: sys progs/init.o
	$(LD) -T user.ld $(SYS_OFILES) progs/init.o -o $(BIN)/$@
