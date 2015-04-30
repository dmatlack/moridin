/**
 * @file assert.h
 *
 */
#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <stddef.h>
#include <kernel/debug.h>
#include <kernel/log.h>
#include <arch/irq.h>

#define panic(_fmt, ...) do {						\
	disable_irqs();							\
									\
	FATAL("[%s:%d %s()] "_fmt,					\
	      __FILE__, __LINE__, __func__, ##__VA_ARGS__);		\
									\
	backtrace();							\
	while (1) continue;						\
} while (0)

#define assert(expression) do {						\
	signed long __condition = (signed long) (expression);		\
									\
	if (__condition)						\
		break;							\
									\
	panic("Failed Assert: %s [%ld]",				\
	      #expression, __condition);				\
} while (0)

#define ASSERT(expression) assert(expression)
#define ASSERT_NOT_NULL(expression) ASSERT(NULL != (expression))

#define ASSERT_BINARY_OPERATOR(_left, _op, _right) do {			\
	signed long __left = (signed long) (_left);			\
	signed long __right = (signed long) (_right);			\
									\
	if (__left _op __right)						\
		break;							\
									\
	panic("Failed Assert: %s [%ld] %s %s [%ld]",			\
	      #_left, __left, #_op, #_right, __right);			\
} while (0)

#define ASSERT_EQUALS(left, right) ASSERT_BINARY_OPERATOR(left, ==, right)
#define ASSERT_NOTEQUALS(left, right) ASSERT_BINARY_OPERATOR(left, !=, right)
#define ASSERT_LESS(left, right) ASSERT_BINARY_OPERATOR(left, <, right)
#define ASSERT_LESSEQ(left, right) ASSERT_BINARY_OPERATOR(left, <=, right)
#define ASSERT_GREATER(left, right) ASSERT_BINARY_OPERATOR(left, >, right)
#define ASSERT_GREATEREQ(left, right) ASSERT_BINARY_OPERATOR(left, >=, right)

#endif /* _ASSERT_H_ */
