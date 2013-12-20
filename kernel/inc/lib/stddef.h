#ifndef _STDDEF_H_
#define _STDDEF_H_

#include <types.h> /* for size_t, ptrdiff_t */

#ifndef NULL
#define NULL ((void *)0)
#endif

#define arraylen(array) (sizeof(array)/sizeof(array[0]))
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

#define KB(n) ((n) * KILOBYTE)
#define MB(n) ((n) * MEGABYTE)
#define GB(n) ((n) * GIGABYTE)

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

static inline void __set_bit(int *word, int index, int bit) {
  *word = (bit == 0) ?
      (*word) & ~(1 << index)
      :
      (*word) | (1 << index);
}
static inline int __get_bit(int word, int index) {
  return (word >> index) & 0x1;
}

#define set_bit(w, i, b) __set_bit((int *) w, (int) i, (int) b)
#define get_bit(w, i)    __get_bit((int) w, (int) i)

/**
 * @brief Return the binary number that contains n ones in a row.
 *
 * e.g.
 *  MASK(4) = 15 = 1111_2
 *  MASK(10) = 1023 = 1111111111_2
 */
#define MASK(n) ((1 << n) - 1)

#endif /* _STDDEF_H_ */
