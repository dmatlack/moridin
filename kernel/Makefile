###############################################################################
#
# Kernel Makefile
#
###############################################################################
include ../config.mk

SRCDIRS = boot          \
          dev           \
          fs            \
          kernel        \
          lib           \
          mm            \
          arch/$(ARCH)

INCDIRS = inc      \
          inc/lib

LINKER_SCRIPT = arch/$(ARCH)/linker.ld

CFILES = $(shell find $(SRCDIRS) -type f -name "*.c")
SFILES = $(shell find $(SRCDIRS) -type f -name "*.S")
OFILES = $(patsubst %.c,%.o,$(CFILES)) $(patsubst %.S,%.o,$(SFILES))

HFILES = $(shell find $(INCDIRS) -type f -name "*.h")
INCLUDES = $(addprefix -I, $(INCDIRS))

CFLAGS += -DKDEBUG

.PHONY: default clean arch kernel

default: kernel

kernel: $(OFILES) $(HFILES) $(LINKER_SCRIPT) symbols
	$(CC) -T $(LINKER_SCRIPT) -o KERNEL.o -ffreestanding -O2 -nostdlib $(OFILES) -lgcc
	nm -n KERNEL.o | tools/inject_symbol_table kernel/symbols.o \
		$(shell tools/symbols_section_offset.sh kernel/symbols.o)
	$(CC) -T $(LINKER_SCRIPT) -o KERNEL.o -ffreestanding -O2 -nostdlib $(OFILES) -lgcc

$(OFILES): arch

#
# Create a symbolic link for architecture-specific header files:
# 		e.g inc/arch -> ../arch/x86/inc
# 
# Note the '..' is needed so that inc/arch knows to look up in the
# parent directory first.
#
arch:
	rm -f inc/arch
	ln -s ../arch/$(ARCH)/inc inc/arch

clean:
	rm -f inc/arch
	rm -f KERNEL.o
	rm -f tools/inject_symbol_table
	rm -f $(OFILES)

symbols: tools/inject_symbol_table.c
	gcc tools/inject_symbol_table.c -o tools/inject_symbol_table

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -DASSEMBLER -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
