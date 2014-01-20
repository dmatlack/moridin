.section .text

#
# void _start(int argc, char **argv);
#
.global _start
_start:
  push $0x0
  movl %esp, %ebp
  
  pushl 8(%ebp)
  pushl 4(%ebp)
  
  call main

exit_not_implemented:
  jmp exit_not_implemented 
