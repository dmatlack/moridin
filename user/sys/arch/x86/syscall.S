###############################################################################
# @file user/sys/arch/x86/syscall.S
# @author David Matlack
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
  #   eax: The system call to execute
  #   ecx: A pointer to data
  # 
  movl 0xC(%ebp), %ecx
  movl 0x8(%ebp), %eax
  int $0x80
  
  leave
  ret
