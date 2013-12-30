/**
 * @file kernel/atomic.h
 */
#ifndef __KERNEL_ATOMIC_H__
#define __KERNEL_ATOMIC_H__

#include <stdint.h>

#ifdef ARCH_X86
#include <arch/x86/atomic.h>

#define atomic_add(ptr, add) __xadd((ptr), (add))
#define atomic_xchg(ptr, new) __xchg((ptr), (new))
#define atomic_testandset(ptr, old, new) __cmpxchg((ptr), (old), (new))

#endif

#endif /* !__KERNEL_ATOMIC_H__ */