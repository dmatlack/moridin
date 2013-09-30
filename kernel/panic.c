/**
 * @file panic.c
 *
 * @brief OH MY GOD!!!!!! WTF!!!!!!!
 *
 * @author David Matlack
 */
#include <stdarg.h>      // for va_list, va_start, va_end
#include <fmt/_printf.h> // for _vprintf
#include <kernel.h>      // for the kernel's kprintf stuff

/* aaaand we'll just borrow that kernel printf state. thanks kprintf */
extern struct printf_state kprintf_state;

int panic(const char *fmt, ...) {
  va_list args;
  int err;

  // here we specify our putchar method
  kprintf_state.putchar = kputchar;

  // now print using the specified putchar!
  va_start(args, fmt);
  err = _vprintf(&kprintf_state, fmt, args);
  va_end(args);

  __asm("cli");
  while (1);

  return err;
}
