/**
 * @file assert.h
 *
 * @author David Matlack
 */
#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <stddef.h>

/**
 * @brief Print a message to the screen, disable interrupts, and then
 * loop endlessly.
 */
int panic(const char *fmt, ...);

#define assert(expression) \
	((void)((expression) ? 0 : (panic("%s:%u: failed assertion `%s'", \
					  __FILE__, __LINE__, #expression), 0)))

#define ASSERT(expression) assert(expression)

#define ASSERT_NOT_NULL(expression) ASSERT(NULL != (expression))

#define ASSERT_EQUALS(left, right)\
  ((void)(\
    (int)(left) == (int)(right) ? \
      0 : \
      panic("%s:%u: failed ASSERT_EQUALS!\n" \
            "   EXPECTED: %s == %s\n" \
            "        GOT: 0x%x != 0x%x", \
            __FILE__, __LINE__, #left, #right, (size_t)left, (size_t)right)\
  ))
      

#endif /* _ASSERT_H_ */
