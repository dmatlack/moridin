###############################################################################
# @file boot/boot.S
#
# The Bare Bones tutorial on osdev served as a starting point for this
# file: http://wiki.osdev.org/Bare_Bones
#
# Much of the source code and layout of this file was taken from the CMU 
# Operating Systems course (15-410) starter code (410kern/boot/head.S).
# I mostly just added documentation and organized some of the code into 
# separate files.
#
# @author David Matlack
###############################################################################
#define ASM_FILE

#include <boot/multiboot.h>
#include <arch/x86/seg.h>

.global lgdt
.global lidt
.global _start

# Declare constants used for creating a multiboot header.
.set MULTIBOOT_HEADER_FLAGS, MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
.set MULTIBOOT_HEADER_CHECKSUM, -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

.include "arch/x86/gdt.S"
.include "arch/x86/tss.S"

.section .multiboot
  .align 4
  .long MULTIBOOT_HEADER_MAGIC
  .long MULTIBOOT_HEADER_FLAGS
  .long MULTIBOOT_HEADER_CHECKSUM

.section .text

init_idt: # (256 entries) x (8 bytes per entry) = 2048 bytes
  .space 2048
init_gdt: # see x86/gdt.S
  SEGDESC_NULL
  SEGDESC_KERNEL_TSS
  SEGDESC_KERNEL_CS
  SEGDESC_KERNEL_DS
  SEGDESC_USER_CS
  SEGDESC_USER_DS
init_tss: # see x86/tss.S
  TSS_SINGLE_TASK

# The linker script specifies _start is the entry point of the kernel. The
# bootloader (e.g. grub) will drop us here.
_start:
  cli                         # disable all interrupts 
  movl $boot_stack_top, %esp  # set esp so that we can run on a stack
  pushl %ebx                  # push the address of the multiboot info struct
  pushl %eax                  # push the multiboot magic value
  movl %esp, %ebp             # save this stack address in %ebp

  movl $init_tss, %eax        # set up the Global Descriptor Table
  subl $init_gdt, %eax
  subl $1, %eax
  pushl %eax
  pushl $init_gdt
  call lgdt
  addl $8, %esp

  movl $init_gdt, %eax        # set up the Interrupt Descriptor Table
  subl $init_idt, %eax
  subl $1, %eax
  pushl %eax
  pushl $init_idt
  call lidt
  addl $8, %esp

  movl $SEGSEL_TSS, %eax      # set up a hardware task
  ltr %ax

  movl %ebp, %esp             # restore the stack
  call mb_entry               # mb_entry(multiboot_magic, multiboot_info)

  # Y U RETURN FROM KERNEL?

  cli                         # disable interrupts
  hlt                         # halt the cpu until the next interrupt arrives
.Lhang:                       # create a local label rather than a real symbol
  jmp .Lhang                  # loop endlessly

###
# lgdt -- Load the Global Descriptor Table
#
# This function taken from the 15-410 starter code. I only added comments.
###
lgdt:
  movl 4(%esp), %eax      # 4(%esp) = address of the gdt
  movl 8(%esp), %ecx      # 8(%esp) = gdt limit (the size of the gdt)
  pushl %eax              # push 4 bytes (addr of gdt)
  pushw %cx               # push 2 bytes (size of gdt)
  lgdt (%esp)             # load the GDT 

  # Do a longjmp to cause a GDT reload and to set the CS register
  ljmp $SEGSEL_KERNEL_CS, $lgdt_reload  

  # set the data segment registers
lgdt_reload:
  movl $SEGSEL_KERNEL_DS, %eax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss
  addl $6, %esp
  ret

###
# lidt -- Load the Interrupt Descriptor Table
#
# This function taken from the 15-410 starter code. I only added comments.
###
lidt:
  movl 4(%esp), %eax    # 4(%esp) is the idt base address
  movl 8(%esp), %ecx    # 8(%esp) is the idt limit/size
  pushl %eax            # push 4 bytes for the base address
  pushw %cx             # push 2 bytes for the limit
  lidt (%esp)           # load the IDT
  addl $6, %esp         # clean up our local stack
  ret

# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start

.section .data

# Allocate room for a small temporary stack
boot_stack_bottom:
  .space 4096
boot_stack_top:

#undef ASM_FILE
