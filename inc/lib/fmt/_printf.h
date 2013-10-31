/**
 * @file _printf.h
 *
 * @brief A putchar-agnostic printf library.
 *
 * @author David Matlack
 */
#ifndef ___PRINTF_H__
#define ___PRINTF_H__

#include <stdarg.h>

#define	_PRINTF_BUFMAX	128

struct printf_state {
  /** @brief internal state used by the print functions */
	char buf[_PRINTF_BUFMAX];
  /** @brief internal state used by the print functions */
	unsigned int index;

  /* @brief the putchar method this version of printf will use */
  int (*putchar)(int c);
};

int _vprintf(struct printf_state *p, const char *fmt, va_list args);

#endif /* ___PRINTF_H__ */

