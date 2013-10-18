###
# @file idt.S
#
# @brief Assembly code needed to the program the Interrupt Descriptor
# Table.
#
# @author David Matlack
###
#define ASM_FILE

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

#undef ASM_FILE
