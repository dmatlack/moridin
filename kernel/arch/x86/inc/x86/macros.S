#
# @file arch/x86/macros.S
#

.macro POP_REGISTERS
  popl %ebx
  movl %ebx, %cr3
  popl %ebx
  movl %ebx, %cr2

  popl %edi
  popl %esi
  popl %ebp
  popl %ebx
  popl %edx
  popl %ecx
  popl %eax

  popl %gs
  popl %fs
  popl %es
  popl %ds
.endm # POP_REGISTERS

.macro PUSH_REGISTERS
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs

  pushl %eax
  pushl %ecx
  pushl %edx
  pushl %ebx
  pushl %ebp
  pushl %esi
  pushl %edi

  movl %cr2, %ebx
  pushl %ebx
  movl %cr3, %ebx
  pushl %ebx
.endm # PUSH_REGISTERS
