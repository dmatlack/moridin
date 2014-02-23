###############################################################################
# 
# config.mk
#
# Project-wide build configurations (compilers, flags, etc.)
#
###############################################################################

ARCH:=x86
ARCHMACRO := ARCH_$(shell echo $(ARCH) | tr '[:lower:]' '[:upper:]')

ifeq ($(ARCH),x86)
CC = i586-elf-gcc
LD = i586-elf-ld
AS = i586-elf-as
else
$(error "Cannot compile for unknown architecture: $(ARCH)")
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

CFLAGS := -g -std=c99 -ffreestanding -D$(ARCHMACRO) $(WARNINGS)

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -DASSEMBLER -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
