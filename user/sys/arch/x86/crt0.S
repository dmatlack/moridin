.section .text

.extern exit

#
# void _start(int argc, char **argv);
#
.global _start
_start:
	push $0x0
	movl %esp, %ebp

	pushl 0xC(%ebp)
	pushl 0x8(%ebp)

	xor %eax, %eax
	call main

	pushl %eax
	call exit

exit_returned:
	jmp exit_returned
