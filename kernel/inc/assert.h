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

void __assert_equals(const char *file, const char *func, int line,
                     int left, const char *leftstr, 
                     int right, const char *rightstr);

#define ASSERT_EQUALS(left, right)\
  __assert_equals(__FILE__, __func__, __LINE__, left, #left, right, #right)

#endif /* _ASSERT_H_ */
