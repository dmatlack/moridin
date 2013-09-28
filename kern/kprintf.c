/**
 * @file kprintf.c
 *
 * @brief The kernel printf function. Used for printing to the
 * console.
 *
 * @author David Matlack
 */
#include <stdarg.h>      // for va_list, va_start, va_end
#include <fmt/_printf.h> // for _vprintf
#include <dev/vga.h>     // for the kernel's putchar method

// statically allocate the state for our kernel's printer
static printf_t kprintf_state;

static int kputchar(int c) {
  vga_putbyte((char) c);
  return c;
}

int kprintf(const char *fmt, ...) {
	va_list	args;
	int err;

  // here we specify our putchar method
  kprintf_state.putchar = kputchar;

  // now print using the specified putchar!
	va_start(args, fmt);
	err = _vprintf(&kprintf_state, fmt, args);
	va_end(args);

	return err;
}
