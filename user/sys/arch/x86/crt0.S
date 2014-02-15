.section .text

#
# void _start(int argc, char **argv);
#
.global _start
_start:
  push $0x0
  movl %esp, %ebp
  
  pushl 0xC(%ebp)
  pushl 0x8(%ebp)
  
  call main

exit_not_implemented:
  jmp exit_not_implemented 
