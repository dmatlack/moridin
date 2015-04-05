###############################################################################
# 
# config.mk
#
# Project-wide build configurations (compilers, flags, etc.)
#
###############################################################################

ifndef ARCH
$(error Must compile for a target architecture. Run 'make ARCH=foo' or \
	set an ARCH environment variable)
endif

ifeq ($(ARCH),x86)
CC := i586-elf-gcc
LD := i586-elf-ld
AS := i586-elf-as
else
$(error Unknown architecture: $(ARCH))
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

CFLAGS := -g -std=c99 -ffreestanding -D__$(ARCH)__ $(WARNINGS)

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -DASSEMBLER -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
