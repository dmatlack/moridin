/**
 * @math.h
 */
#ifndef __LIB_MATH_H__
#define __LIB_MATH_H__

#include <stddef.h>

static inline bool check_overlap(unsigned int a_start, unsigned int a_len, 
                                 unsigned int b_start, unsigned int b_len) {
  return (a_start >= b_start && a_start <= b_start + b_len) ||
         (b_start >= a_start && b_start <= a_start + a_len);
}

static inline unsigned __umin(unsigned a, unsigned b) {
  return a < b ? a : b;
}
#define umin(a, b) __umin((unsigned) (a), (unsigned) (b))

static inline unsigned __smin(int a, int b) {
  return a < b ? a : b;
}
#define smin(a, b) __smin((unsigned) (a), (unsigned) (b))

#endif /* !__LIB_MATH_H__ */
