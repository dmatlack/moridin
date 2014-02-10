.global syscall
syscall:
  push %ebp
  movl %esp, %ebp
  
  #
  #   eax: The system call to execute
  #   ecx: A pointer to data
  # 
  movl $0xDEADBABE, %ecx
  movl $0x1,        %eax
  int $0x80
  
  leave
  ret
