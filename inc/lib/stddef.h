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

/**
 * @brief rounds up to the nearest multiple of size
 * @param size the size
 * @param val the value to round up
 * @return the rounded up value
 */
#define CEIL(size, val) \
  (((((unsigned) (val)) + ((unsigned) (size) - 1)) / (size)) * (size))

/**
 * @brief floors down to the nearest multiple of size
 * @param size the size
 * @param val the value to round down
 * @return the rounded down value
 */
#define FLOOR(size, val) \
  ((((unsigned) (val)) / (size)) * (size))

#define SET_BIT(word, index, bit) \
  (bit == 0) ?                    \
    word & ~(1 << index)          \
    :                             \
    word | (1 << index)

#endif /* _STDDEF_H_ */
