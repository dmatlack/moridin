###
# @file arch/x86/syscall.S
#
# @brief System call entry point on the x86 architecture.
#
# WARNING: WORK IN PROGRESS
# -------------------------
#
# System calls are entered by invoking a software interrupt (e.g. INT 0x80).
# System calls invoked this way have the following ABI:
#
# Call
#   eax: The system call to execute
#   ecx: first argument
#   edx: second argument
#   esi: third argument
#   edi: fourth argument
#
# Return
#   eax: The return value of the system call
#
###
#include <arch/x86/macros.S>

.extern syscall_table
.extern set_thread_regs
.extern return_from_syscall

.global __syscall_entry
__syscall_entry:

  PUSH_REGISTERS

  #
  # push all the possible arguments onto the stack. it doesn't matter if
  # the system call uses < 4 arguments. it will just ignore the bogus
  # values we've pushed
  #
  pushl %edi
  pushl %esi
  pushl %edx
  pushl %ecx

  pushl %eax              # save the syscall number temporarily

  leal 0x14(%esp), %eax   # load the address of the registers
  pushl %eax              # push that address on the stack
  call set_thread_regs    # set regs pointer in the thread struct
  addl $4, %esp           # restore the stack

  popl %eax               # restore the syscall number

  #
  # eax is the system call we want to execute. we lookup the eax'th system
  # call in the syscall_table and call that address.
  #
  # TODO: sanity check eax to make sure not negative or out of bounds
  #
  movl $syscall_table, %ecx
  movl (%ecx, %eax, 0x4), %ecx
  call *%ecx
  
  popl %ecx
  popl %ecx
  popl %ecx
  popl %ecx

  pushl %eax
  call return_from_syscall
