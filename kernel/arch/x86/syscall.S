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
# @author David Matlack
###

.extern syscall_table

.global x86_syscall
x86_syscall:

  #
  # push all the possible arguments onto the stack. it doesn't matter if
  # the system call uses < 4 arguments. it will just ignore the bogus
  # values we've pushed
  #
  pushl %edi
  pushl %esi
  pushl %edx
  pushl %ecx

  #
  # eax is the system call we want to execute. we lookup the eax'th system
  # call in the syscall_table and call that address.
  #
  # TODO: sanity check eax to make sure not negative or out of bounds
  #
  movl syscall_table, %ecx
  leal (%ecx, %eax, 0x4), %ecx
  call %ecx
  
  #
  # restore the stack to the iret stack
  #
  popl %ecx
  popl %ecx
  popl %ecx
  popl %ecx

  #
  # return to userspace
  #
  iret
