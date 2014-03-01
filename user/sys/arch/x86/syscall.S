###############################################################################
# @file user/sys/arch/x86/syscall.S
#
#
# This is the user space entrance point into the kernel. Since protection is
# a thing of hardware, entering kernel space is a highly architecture dependent
# process. This is the entrace to the kernel for x86 architectures.
#
###############################################################################

.global __syscall
__syscall:
  push %ebp
  movl %esp, %ebp

  #
  # save the callee-saved registers we need to use
  #
  pushl %esi
  pushl %edi
  
  #
  #   eax: The system call to execute
  #   ecx: first argument
  #   edx: second argument
  #   esi: third argument
  #   edi: fourth argument
  # 
  movl  0x8(%ebp), %eax
  movl  0xC(%ebp), %ecx
  movl 0x10(%ebp), %edx
  movl 0x14(%ebp), %esi
  movl 0x18(%ebp), %edi

  #
  # software trap into the kernel
  #
  int $0x80

  #
  # restore callee-saved registers
  #
  popl %edi
  popl %esi
  
  leave
  ret
