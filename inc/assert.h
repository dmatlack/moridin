/**
 * @file assert.h
 *
 * @author David Matlack
 */
#ifndef _ASSERT_H_
#define _ASSERT_H_

#define assert(expression) \
	((void)((expression) ? 0 : (panic("%s:%u: failed assertion `%s'", \
					  __FILE__, __LINE__, #expression), 0)))

#endif /* _ASSERT_H_ */
