#
#
# Kernel Makefile
#
#

PWD = $(shell pwd)

AUTOSTART := $(shell rm -f KERNEL.S)

###############################################################################
# Project Directory/Files
###############################################################################

# directories with source code
PROJDIRS := arch boot lib dev kernel debug mm

# source files by type
CFILES = $(shell find $(PWD)/$(PROJDIRS) -type f -name "*.c")
HFILES = $(shell find $(PWD)/$(PROJDIRS) -type f -name "*.h")
SFILES = $(shell find $(PWD)/$(PROJDIRS) -type f -name "*.S")

OBJFILES = $(patsubst %.c,%.o,$(CFILES)) $(patsubst %.S,%.o,$(SFILES))


###############################################################################
# Compiler Options
###############################################################################
CC = i586-elf-gcc # compiler
AS = i586-elf-as  # assembler
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

CFLAGS := -g -std=c99 -ffreestanding -DARCH_X86 $(WARNINGS) -DKDEBUG
INCLUDES := -I$(PWD)/inc/lib -I$(PWD)/inc

###############################################################################
# Targets
###############################################################################
.PHONY: all clean 

all: $(OBJFILES) $(HFILES) linker.ld
	$(CC) -T linker.ld -o KERNEL.o -ffreestanding -O2 -nostdlib $(OBJFILES) -lgcc
	objdump -D KERNEL.o > KERNEL.S

clean:
	rm -rf KERNEL.S
	rm -rf KERNEL.o
	rm -rf $(OBJFILES)

###############################################################################
# General Make Rules for filetypes
###############################################################################

%.d: %.S
	$(CC) $(CFLAGS) -DASSEMBLER $(INCLUDES) -M -MP -MF $@ -MT $(<:.S=.o) $<

%.d: %.s
	@echo "You should use the .S file extension rather than .s"
	@echo ".s does not support precompiler directives (like #include)"
	@false

%.d: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -M -MP -MF $@ -MT $(<:.c=.o) $<

%.o: %.S
	$(CC) $(CFLAGS) -DASSEMBLER $(INCLUDES) -c -o $@ $<
	#$(OBJCOPY) -R .comment -R .note $@ $@

%.o: %.s
	@echo "You should use the .S file extension rather than .s"
	@echo ".s does not support precompiler directives (like #include)"
	@false

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
	#$(OBJCOPY) -R .comment -R .note $@ $@

###############################################################################
