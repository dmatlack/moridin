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

.global kernel_idt
.global kernel_gdt
.global kernel_tss

.global boot_page_dir
.global boot_stack_bottom
.global boot_stack_top
.global lgdt
.global lidt
.global _start

# Declare constants used for creating a multiboot header.
.set MULTIBOOT_HEADER_FLAGS, MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
.set MULTIBOOT_HEADER_CHECKSUM, -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

#
# 0x100000
#
.section .multiboot
multiboot_header:
  .align 4
  .long MULTIBOOT_HEADER_MAGIC
  .long MULTIBOOT_HEADER_FLAGS
  .long MULTIBOOT_HEADER_CHECKSUM

.section .intel
#
# 0x10000c
#
kernel_idt: # (256 entries) x (8 bytes per entry) = 2048 bytes
  .space 2048

#
# 0x10080c
#
kernel_gdt:
  #
  # SEGDESC_NULL
  #
  .long 0x00000000
  .long 0x00000000
  #
  # SEGDESC_TSS
  #   base:  0x10083c (kernel_tss)
  #   limit: 0x68 - 1 byte = 0x67
  #   dpl:   0
  #
  .long 0x083c0067
  .long 0x00008910
  #
  # SEGDESC_KERNEL_CS
  #   base:  0x0000
  #   limit: 0xffff
  #   dpl:   0
  #   type:  R/E
  #
  .long 0x0000ffff
  .long 0x00cf9b00
  #
  # SEGDESC_KERNEL_DS
  #   base:  0x0000
  #   limit: 0xffff
  #   dpl:   0
  #   type:  R/W
  #
  .long 0x0000ffff
  .long 0x00cf9300
  #
  # SEGDESC_USER_CS
  #   base:  0x0000
  #   limit: 0xffff
  #   dpl:   3
  #   type:  R/E
  #
  .long 0x0000ffff
  .long 0x00cffb00
  #
  # SEGDESC_USER_DS
  #   base:  0x0000
  #   limit: 0xffff
  #   dpl:   3
  #   type:  R/W
  #
  .long 0x0000ffff
  .long 0x00cff200

#
# 0x10083c
#
kernel_tss:
  # TSS_SINGLE_TASK
  .space 8
  .word 0x0018 # ss0 = SEGSEL_KERNEL_DS in <arch/x86/seg.h>
  .space 92
  .word 0x0068 # IOBP = sizeof(TSS) = 104 = 0x68

.section .text

# The linker script specifies _start is the entry point of the kernel. The
# bootloader (e.g. grub) will drop us here.
_start:
  cli                         # disable all interrupts 
  movl $boot_stack_top, %esp  # set esp so that we can run on a stack
  pushl %ebx                  # push the address of the multiboot info struct
  pushl %eax                  # push the multiboot magic value
  movl %esp, %ebp             # save this stack address in %ebp

  movl $kernel_tss, %eax      # set up the Global Descriptor Table
  subl $kernel_gdt, %eax
  subl $1, %eax
  pushl %eax
  pushl $kernel_gdt
  call lgdt
  addl $8, %esp

  movl $kernel_gdt, %eax      # set up the Interrupt Descriptor Table
  subl $kernel_idt, %eax
  subl $1, %eax
  pushl %eax
  pushl $kernel_idt
  call lidt
  addl $8, %esp

  movl $SEGSEL_TSS, %eax      # set up a hardware task
  ltr %ax

  call boot_paging_init

  movl %ebp, %esp             # restore the stack
  call mb_entry               # mb_entry(multiboot_magic, multiboot_info)

  # Y U RETURN FROM KERNEL?

  cli                         # disable interrupts
  hlt                         # halt the cpu until the next interrupt arrives
.Lhang:                       # create a local label rather than a real symbol
  jmp .Lhang                  # loop endlessly

###
# boot_paging_init -- Set up the boot_page_dir to direct map the first
# 16 MB of memory.
###
.global boot_paging_init
boot_paging_init:
  #
  # Enable 4 MB pages by setting the PSE flag of cr4
  #
  movl %cr4, %eax
  orl $0x10, %eax
  movl %eax, %cr4

  #
  # Set up the page directory
  #
  movl $boot_page_dir, %eax    # eax now points to the page directory
  movl $0x00000083, 0x0(%eax)  # Map [0, 4MB]
  movl $0x00400083, 0x4(%eax)  # Map [4MB, 8MB]
  movl $0x00800083, 0x8(%eax)  # Map [8MB, 12MB]
  movl $0x00C00083, 0xC(%eax)  # Map [12MB, 16MB]

  # NOTE: to implement a high half kernel, we can extend the above
  # functionality and map [3 GB, 3 GB + 16 MB] to [0 MB, 16 MB]
  # (which is what Linux does). This makes it easy to still address
  # things like the the video memory because you can still use 
  # physical addresses

  #
  # Set cr3 to set the page directory, then enable paging in cr0
  #
  movl %eax, %cr3
  movl %cr0, %eax
  orl $0x80000000, %eax
  movl %eax, %cr0

  ret

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

#
# Create a page directory to use to enable paging early in the boot
# process.
#
.section .bpgdir
boot_page_dir:
  .space 4096

.section .data

# Allocate room for a small temporary stack
boot_stack_bottom:
  .space 4096
boot_stack_top:

#undef ASM_FILE
