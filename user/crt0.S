.section .text

.global _start
#
# void _start(int argc, char **argv);
#
_start:
  push $0x0
  movl %esp, %ebp
  
  pushl 8(%ebp)
  pushl 4(%ebp)
  
  call main

exit_not_implemented:
  jmp exit_not_implemented 
