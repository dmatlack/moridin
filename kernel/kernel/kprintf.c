/**
 * @file kprintf.c
 *
 * @brief The kernel printf function. Used for printing to the
 * console.
 *
 */
#include <stdarg.h>      // for va_list, va_start, va_end
#include <fmt/_printf.h> // for _vprintf

#include <kernel/debug.h>
#include <kernel/irq.h>

// statically allocate the state for our kernel's printer
struct printf_state kprintf_state;
void (*__kputchar)(char c);

void kputchar_set(void (*kpc)(char)) {
  __kputchar = kpc;
}

int kputchar(int c) {
  __kputchar((char) c);
  return c;
}

int kprintf(const char *fmt, ...) {
  va_list args;
  int err;

  // here we specify our putchar method
  kprintf_state.putchar = kputchar;

  // now print using the specified putchar!
  va_start(args, fmt);
  err = _vprintf(&kprintf_state, fmt, args);
  va_end(args);

  return err;
}
