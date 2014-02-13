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
#   eax: The system call to invoke (integer identifier)
#   ecx: The address in user memory of the system call arguments
#
# Return
#   eax: The return value of the system call
#
# @author David Matlack
###

.extern __syscall

.global x86_syscall
x86_syscall:
  #
  # FIXME what registers should we save/restore here?
  #
  pushl %ecx
  pushl %eax
  call __syscall
  popl %ecx
  popl %ecx
  iret
