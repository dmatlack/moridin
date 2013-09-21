/**
 * @file x86/io.h
 *
 * @brief Utility functions for communicating with IO ports.
 *
 * @author David Matlack
 */
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
