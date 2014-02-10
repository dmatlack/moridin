#
# int syscall(int s, void *arg)
#
.global syscall
syscall:
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
