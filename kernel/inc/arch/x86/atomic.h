/**
 * @file arch/x86/atomic.h
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

/**
 * @brief Atomically perform: *(ptr) = value
 *
 * @return the old value of *(ptr)
 */
int __xchg(int *ptr, int new);

/**
 * @brief Atomically perform: if (*(ptr) == (cmp)) { *(ptr) = new; }
 *
 * @return the old value of *(ptr)
 */
int __cmpxchg(int *ptr, int old, int new);

#endif /* !__ARCH_X86_ATOMIC_H__ */
