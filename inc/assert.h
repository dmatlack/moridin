/**
 * @file assert.h
 *
 * @author David Matlack
 */
#ifndef _ASSERT_H_
#define _ASSERT_H_

/**
 * @brief Print a message to the screen, disable interrupts, and then
 * loop endlessly.
 */
int panic(const char *fmt, ...);

#define assert(expression) \
	((void)((expression) ? 0 : (panic("%s:%u: failed assertion `%s'", \
					  __FILE__, __LINE__, #expression), 0)))

#define ASSERT(expression) assert(expression)

#endif /* _ASSERT_H_ */
