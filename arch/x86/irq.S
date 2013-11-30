# Generate an IRQ, jumping us into the register interrupt service routine
# specified in an entry in the IDT.
.global generate_irq
generate_irq:
  push %ebp
  mov %esp, %ebp
  
  # The "int" instruction can only take an immediate as it's argument. So
  # we use self modifying code to overwrite the operand in memory.
  movb 0x8(%ebp), %al
  movl $int_x,%ecx
  movb %al, 0x1(%ecx)

int_x:
  int $0
  
  leave
  ret
