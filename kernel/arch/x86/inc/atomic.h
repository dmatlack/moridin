/**
 * @file arch/atomic.h
 */
#ifndef __ARCH_X86_ATOMIC_H__
#define __ARCH_X86_ATOMIC_H__

#include <stdint.h>

/**
 * @brief Atomically perform: *(ptr) += add
 *
 * @return the old value of *(ptr)
 */
int __xadd(int *ptr, int add);
#define atomic_add(ptr, add) __xadd((ptr), (add))
#define atomic_inc(ptr) __xadd((ptr), (1))
#define atomic_dec(ptr) __xadd((ptr), (-1))

/**
 * @brief Atomically perform: *(ptr) = value
 *
 * @return the old value of *(ptr)
 */
int __xchg(int *ptr, int new);
#define atomic_xchg(ptr, new) __xchg((ptr), (new))

/**
 * @brief Atomically perform: if (*(ptr) == (cmp)) { *(ptr) = new; }
 *
 * @return the old value of *(ptr)
 */
int __cmpxchg(int *ptr, int old, int new);
#define atomic_testandset(ptr, old, new) __cmpxchg((ptr), (old), (new))

static inline int atomic_get(int *ptr)
{
	return *ptr;
}

#endif /* !__ARCH_X86_ATOMIC_H__ */
