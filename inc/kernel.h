/**
 * @file kernel.h
 *
 * @brief All stardard kernel functions and includes.
 *
 * @author David Matlack
 */
#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Print a character to the screen.
 */
int kputchar(int c);

/**
 * @brief Formatted printing to the console.
 */
int kprintf(const char *fmt, ...);

/**
 * @brief Print a message to the screen, disable interrupts, and then
 * loop endlessly.
 */
int panic(const char *fmt, ...);

#define must_succeed(expression)  \
	((void)((expression) == 0 ? 0 : (panic("%s:%u: failed assertion `%s'", \
					  __FILE__, __LINE__, #expression), 0)))

#endif /* __KERNEL_H__ */
