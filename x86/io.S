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

.global ind
ind:
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
