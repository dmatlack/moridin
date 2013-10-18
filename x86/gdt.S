###
# @file gdt.h
#
# @brief Defines for the global descriptor table used in the kernel.
#
# The Global Descriptor Table holds the segment descriptors used by
# both the user and the kernel. Consider the segment selectors CS,
# DS, ES, FS, and GS. Each is a reference to an element in the GDT,
# called a segment descriptor.
#
# A segment descriptor contains information about a memory segment 
# such as base address, limit and privileges.
# 
# @author David Matlack
###
#define ASM_FILE

###
# SEGMENT DESCRIPTOR
#  See Intel IA-32 System Programming Guide (Section 3.4.3)
#
#  An 8-byte entry in the Global Descriptor Table. Contains all the 
#  necessary information used to describe a segment.
#
#  Field | Bits | Desc
#  ------+------+-------------------------------------------------------------
#  AVL   | 1    | Avaailable for use by system software
#  BASE  | 32   | Segment base address
#  D/B   | 1    | Default operation size (0 = 16 bit, 1 = 32 bit)
#  DPL   | 2    | Descriptor priveledge level
#  G     | 1    | Granularity (0 = (LIMIT *= 1 byte), 1 = (LIMIT *= 4 Kbyte)
#  LIMIT | 20   | The length of the segment (see G)
#  P     | 1    | Segment Present
#  S     | 1    | Descriptor Type (0 = system, 1 = code or data)
#  TYPE  | 4    | Segment Type
#  0     | 1    | must be zero
#  ---------------------------------------------------------------------------
#         = 64
#
#  We are using a flat memory model in the kernel. Thus every segment 
#  selector has a base address of 0x0, a limit of 0xfffff and a granularity
#  of 1. Thus every segment starts at 0 and ranges for 4 Gigabytes.
#
#  We have 4 total segments. 2 for the user and 2 for the kernel:
#
#    We separate user and kernel segments so that the kernel is protected 
#    from the user tampering with their data.
#
#    We use 2 segments for the user and kernel (CS and DS) so we can have
#    an executable segment for code and a read/write segment for data.
#
# The following handcrafted, magical values, were taken from the 15-410
# Operating Systems course at Carnegie Mellon University starter code.
###
.macro SEGDESC_NULL
    .long 0x00000000
    .long 0x00000000
.endm

.macro SEGDESC_KERNEL_TSS
    .long 0x09300067
    .long 0x00008910
.endm

.macro SEGDESC_KERNEL_CS
    .long 0x0000ffff
    .long 0x00cf9b00
.endm

.macro SEGDESC_KERNEL_DS
    .long 0x0000ffff
    .long 0x00cf9300
.endm

.macro SEGDESC_USER_CS
    .long 0x0000ffff
    .long 0x00cffb00
.endm

.macro SEGDESC_USER_DS
    .long 0x0000ffff
    .long 0x00cff200
.endm

#undef ASM_FILE
