###
# Makefile
#
# by David Matlack
###
PWD = $(shell pwd)

###############################################################################
# Project Directory/Files
###############################################################################

# directories with source code
PROJDIR := boot lib dev x86 kernel debug

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


CFLAGS := -g -std=c99 -ffreestanding $(WARNINGS)
INCLUDES := -I$(PWD)/inc/lib -I$(PWD)/inc

###############################################################################
# Targets
###############################################################################
.PHONY: all clean 

all: $(OBJFILES) $(HFILES) linker.ld boot/grub.cfg
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib $(OBJFILES) -lgcc
	mkdir -p isodir
	mkdir -p isodir/boot
	cp myos.bin isodir/boot/myos.bin
	mkdir -p isodir/boot/grub
	cp boot/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

clean:
	rm -rf isodir
	rm -rf $(OBJFILES)
	rm -rf myos.bin
	rm -rf myos.iso

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
