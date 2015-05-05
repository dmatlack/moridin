/**
 * @file kernel/test.h
 */
#ifndef __KERNEL_TEST_H__
#define __KERNEL_TEST_H__

#include <mm/kmalloc.h>
#include <kernel/log.h>

typedef void (*test_f)(void);

extern char ktest_start[], ktest_end[];

#define __test __attribute__((__section__(".test"))) __attribute__((unused))

#define BEGIN_TEST(_name)                                                     \
void _name(void);                                                             \
                                                                              \
static __test test_f __##_name##_ptr = _name;                                 \
                                                                              \
void _name(void) {                                                            \
	TRACE();                                                              \
	unsigned long __kmalloc_bytes_begin = kmalloc_bytes_used();           \


#define END_TEST                                                              \
	if (__kmalloc_bytes_begin != kmalloc_bytes_used()) {                  \
		panic("MEMORY LEAK: %d bytes lost.",                          \
		      kmalloc_bytes_used() - __kmalloc_bytes_begin);          \
	}                                                                     \
}

static inline void _TEST_KERNEL_(void)
{
	test_f *functions, *f;

	TRACE();

	functions = (test_f *) (ktest_start + 1);
	for (f = functions; f < (test_f *) ktest_end; f++) {
		(*f)();
	}
}

#endif /* !__KERNEL_TEST_H__ */

