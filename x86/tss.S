###
# @file tss.S
#
# @brief Defines for the TSS (Task State Segment).
#
# The TSS (Task State Segment) is a special data structure for x86 processors
# which holds information about a hardware tasks. This kernel does not 
# implement hardware multitasking, but we still need a TSS.
#
# The TSS contains all information needed to manages threads in hardware 
# (all general purpose registers, segment registers, etc.). We only care
# about esp0 and ss0 though.
#
# esp0 -- Holds the address of the stack we will use when a system call
#         (int 0x80) occurs.
# ss0  -- Holds the kernel data segment descriptor (or selector?)
#
# reference: - http://wiki.osdev.org/Task_State_Segment
#            - Intel IA-32 System Programming Guide Section 6.2.1
#
# @author David Matlack
###

# NOTE: We won't set esp0 at this point (in the TSS). We don't need to set it
#       until we have a user process that actually could make system calls.

# there's some little endian magic going on in here...
.macro TSS_SINGLE_TASK
  .space 8
  .word 0x0018 # ss0 = SEGSEL_KERNEL_DS in <x86/seg.h>
  .space 92
  .word 0x0068 # IOBP = sizeof(TSS) = 104 = 0x68
.endm
