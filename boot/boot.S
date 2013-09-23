#
# @file boot/boot.S
#
# This file is heavily based off of the Bare Bones tutorial at:
#   http://wiki.osdev.org/Bare_Bones
#
# @author David Matlack
#

# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Allocate room for a small temporary stack
.section .bootstrap_stack
stack_bottom:
  .space 4096
stack_top:

.section .text

# Declare a header as in the Multiboot Standard. The bootloader will 
# search for this magic sequence and recognize us as a multiboot
# kernel
.section .multiboot
  .align 4
  .long MAGIC
  .long FLAGS
  .long CHECKSUM

# The linker script specifies _start is the entry point of the kernel. The
# bootloader (e.g. grub) will drop us here.
.global _start
_start:
    cli                         # disable all interrupts 
    movl $stack_top, %esp       # set esp so that we can run on a stack
    call kernel_main            # enter the kernel (C code entry point)

    # RETURNED FROM KERNEL MAIN = BAD

    cli                         # disable interrupts
    hlt                         # halt the cpu until the next interrupt arrives
.Lhang:                         # create a local label rather than a real symbol
    jmp .Lhang                  # loop endlessly

# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start

.section .data
