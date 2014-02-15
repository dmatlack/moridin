###############################################################################
# 
# config.mk
#
# Project-wide build configurations (compilers, flags, etc.)
#
###############################################################################

ARCH := ARCH_X86

ifeq ($(ARCH),ARCH_X86)
CC = i586-elf-gcc
LD = i586-elf-ld
AS = i586-elf-as
endif

WARNINGS := \
			-Wall \
			-Wextra \
			-Wshadow \
			-Wcast-align \
			-Wredundant-decls \
			-Wnested-externs \
			-Winline \
			-Wuninitialized \
			-Werror

CFLAGS := -g -std=c99 -ffreestanding -D$(ARCH) $(WARNINGS)

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -DASSEMBLER -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
