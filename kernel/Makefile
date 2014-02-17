###############################################################################
#
# Kernel Makefile
#
###############################################################################
include ../config.mk

SRCDIRS = boot      \
          debug     \
          dev       \
          fs        \
          kernel    \
          lib       \
          mm

ifeq ($(ARCH),ARCH_X86)
SRCDIRS += arch/x86
endif

INCDIRS = inc      \
          inc/lib

CFILES = $(shell find $(SRCDIRS) -type f -name "*.c")
SFILES = $(shell find $(SRCDIRS) -type f -name "*.S")
OFILES = $(patsubst %.c,%.o,$(CFILES)) $(patsubst %.S,%.o,$(SFILES))

HFILES = $(shell find $(INCDIRS) -type f -name "*.h")
INCLUDES = $(addprefix -I, $(INCDIRS))

CFLAGS += -DKDEBUG

.PHONY: default clean 

default: kernel
	
kernel: $(OFILES) $(HFILES) linker.ld
	$(CC) -T linker.ld -o KERNEL.o -ffreestanding -O2 -nostdlib $(OFILES) -lgcc
	objdump -Dg KERNEL.o > KERNEL.S

clean:
	rm -rf KERNEL.S
	rm -rf KERNEL.o
	rm -rf $(OFILES)

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -DASSEMBLER -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
