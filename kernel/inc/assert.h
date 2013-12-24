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

#define ASSERT_FUNCTION_PROTOTYPE(_name)\
    void _name(const char *file, const char *func, int line,\
                 int left, const char *leftstr,\
                 int right, const char *rightstr)

ASSERT_FUNCTION_PROTOTYPE(__assert_equals);
ASSERT_FUNCTION_PROTOTYPE(__assert_notequals);
ASSERT_FUNCTION_PROTOTYPE(__assert_less);
ASSERT_FUNCTION_PROTOTYPE(__assert_lesseq);
ASSERT_FUNCTION_PROTOTYPE(__assert_greater);
ASSERT_FUNCTION_PROTOTYPE(__assert_greatereq);

#define ASSERT_EQUALS(left, right)\
  __assert_equals(__FILE__, __func__, __LINE__, left, #left, right, #right)

#define ASSERT_NOTEQUALS(left, right)\
  __assert_notequals(__FILE__, __func__, __LINE__, left, #left, right, #right)

#define ASSERT_LESS(left, right)\
  __assert_less(__FILE__, __func__, __LINE__, left, #left, right, #right)

#define ASSERT_LESSEQ(left, right)\
  __assert_lesseq(__FILE__, __func__, __LINE__, left, #left, right, #right)

#define ASSERT_GREATER(left, right)\
  __assert_greater(__FILE__, __func__, __LINE__, left, #left, right, #right)

#define ASSERT_GREATEREQ(left, right)\
  __assert_greatereq(__FILE__, __func__, __LINE__, left, #left, right, #right)

#endif /* _ASSERT_H_ */
