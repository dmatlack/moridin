/**
 * @file dprintf.c
 *
 * @brief Debug printing utility.
 *
 * @author David Matlack
 */
#include <stdarg.h>
#include <fmt/_printf.h>
#include <debug/bochs.h>
#include <debug.h>

struct printf_state dprintf_state;

int dputchar(int c) {
  bochs_putchar((char) c); 
  return c;
}

int dprintf(const char *fmt, ...) {
  va_list args;
  int err;

  // here we specify our putchar method
  dprintf_state.putchar = dputchar;

  // now print using the specified putchar!
  va_start(args, fmt);
  err = _vprintf(&dprintf_state, fmt, args);
  va_end(args);

  return err;
}
