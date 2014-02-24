###
# @file arch/x86.S
#
# @brief Assembly routines needed to program the x86 architecture.
###
#include <arch/seg.h>

.global outb
outb:
	movw 4(%esp), %dx
	movb 8(%esp), %al
	out %al, %dx
	ret

.global outw
outw:
	movw 4(%esp), %dx
	movw 8(%esp), %ax
	out %ax, %dx
	ret

.global outl
outl:
	movw 4(%esp), %dx
	movl 8(%esp), %eax
	out %eax, %dx
	ret

.global inb
inb:
	xorl %eax, %eax
	movw 4(%esp), %dx
	in %dx, %al
	ret

.global inw
inw:
	xorl %eax, %eax
	movw 4(%esp), %dx
	in %dx, %ax
	ret

.global inl
inl:
	xorl %eax, %eax
	movw 4(%esp), %dx
	in %dx, %eax
	ret

.global iodelay
iodelay:
    pushl %eax
    inb $0x80,%al
    inb $0x80,%al
    popl %eax
    ret

.global idt_get_base
idt_get_base:
    subl $6, %esp       # the IDTR is a 6 byte (48-bit) register
    sidt (%esp)         # read the IDTR onto the stack
    movl 2(%esp), %eax  # return the upper 4 bytes (the address of the idt) 
    addl $6, %esp       # clean up the stack
    ret

.global idt_get_limit
idt_get_limit:
    subl $6, %esp       # the IDTR is a 6 byte (48-bit) register
    sidt (%esp)         # read the IDTR onto the stack
    movw (%esp), %ax    # return the lower 2 bytes (the limit of the idt) 
    addl $6, %esp       # clean up the stack
    ret

# Generate an IRQ, jumping us into the register interrupt service routine
# specified in an entry in the IDT.
.global __int
__int:
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

.global get_cr0
get_cr0:
    movl %cr0, %eax
    ret

.global get_cr2
get_cr2:
    movl %cr2, %eax
    ret

.global get_cr3
get_cr3:
    movl %cr3, %eax
    ret

.global get_cr4
get_cr4:
    movl %cr4, %eax
    ret

.global get_eflags
get_eflags:
  push %ebp
  mov %esp, %ebp

  pushf
  pop %eax

  leave
  ret
    

# cr0 is special: change must be followed
# by a "far jump" (thanks, nwf).
.global set_cr0
set_cr0:
    movl 4(%esp), %eax   # fetch param
    movl %eax, %cr0      # set %cr0
    pushw %cs            # 16 bits of %cs
    pushl $set_cr0_cont  # 32 bits of %eip
    ljmp *(%esp)         # load %cs:%eip from stack
set_cr0_cont:
    addl $6, %esp        # undo pushes
    ret

.global set_cr2
set_cr2:
    movl 4(%esp), %eax
    movl %eax, %cr2
    ret

.global set_cr3
set_cr3:
    movl 4(%esp), %eax
    movl %eax, %cr3
    ret

.global set_cr4
set_cr4:
    movl 4(%esp), %eax
    movl %eax, %cr4
    ret

.global get_esp
get_esp:
    movl %esp, %eax
    ret

.global enable_irqs
enable_irqs:
  sti
  ret

.global disable_irqs
disable_irqs:
  cli
  ret

.extern kernel_tss
.global set_esp0
set_esp0:
  movl 4(%esp), %eax
  movl $kernel_tss, %ecx          # exc = kernel_tss
  movl %eax, 4(%ecx)              # set esp0
  movw $SEGSEL_KERNEL_DS, 8(%ecx) # set ss0
  ret 

.global restore_registers
restore_registers:

  popl %eax         # pop off the return address, we don't return from this

  #
  # control registers
  #
  popl %eax         # cr4
  popl %eax         # cr3
  movl %eax, %cr3
  popl %eax         # cr2
  movl %eax, %cr2
  popl %eax         # cr0


  #
  # general purpose registers
  #
  popa
  
  #
  # USE OF GENERAL PURPOSE REGISTERS IS NOT ALLOWED AFTER THIS POINT!
  #

  #
  # data segment registers
  #
  popl %gs
  popl %fs
  popl %es
  popl %ds

  #
  # All that's left is the iret stack, so we'll iret to it
  #
  iret

#
# void jump_stacks(uint32_t stack, void (*func)(void *), void *arg);
#
# Run function <func> with arg <arg> on the stack pointed to by
# <stack> (the top of the stack).
#
# NOTE: <func> MUST NEVER RETURN!!!!!!
#
.global jump_stacks
jump_stacks:
  push %ebp
  mov %esp, %ebp

  movl 0x8(%ebp), %eax         # the top of the new stack

  movl 0x10(%ebp), %ecx        # ecx = argument to <func>
  movl %ecx, -0x4(%eax)        # push ecx at top of new stack
  movl $0xDEADBEEF, -0x8(%eax) # push bogus return address of <func>

  movl 0xC(%ebp), %ecx         # save the address of <func> in ecx
  
  movl $0xDEADBABE, %ebp       # load a bogus old stack frame for <func>
  leal -0x8(%eax), %esp        # point esp to the new stack

  jmp %ecx                     # jmp to <func>
