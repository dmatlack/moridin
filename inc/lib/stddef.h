#ifndef _STDDEF_H_
#define _STDDEF_H_

#include <types.h> /* for size_t, ptrdiff_t */

#ifndef NULL
#define NULL ((void *)0)
#endif

#define offsetof(stype,field)	((size_t)(&((stype*)NULL)->field))

/**
 * @brief return the largest value <= x that is a multiple of N.
 */
#define ALIGN_DOWN(x, N) ((x) / (N) * (N))

/**
 * @brief return the smallest value >= x that is a multiple of N.
 */
#define ALIGN_UP(x, N) (((x) + ((N)-1)) / (N) * (N))

#define KILOBYTE 0x400
#define MEGABYTE 0x100000
#define GIGABYTE 0x40000000

#endif /* _STDDEF_H_ */
