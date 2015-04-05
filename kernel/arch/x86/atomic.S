###
# @file arch/x86/atomic.S
#
# @brief Atomic operations in the x86 architecture.
###

.global __xadd
__xadd:
	movl 4(%esp), %edx
	movl 8(%esp), %eax
	xaddl %eax, (%edx)
	ret

.global __xchg
__xchg:
	movl 4(%esp), %edx
	movl 8(%esp), %eax
	xchgl %eax, (%edx)
	ret

.global __cmpxchg
__cmpxchg:
	movl  4(%esp), %edx
	movl  8(%esp), %eax
	movl 12(%esp), %ecx
	cmpxchgl %ecx, (%edx)
	ret
