/**
 * @file _printf.c
 *
 * @brief A putchar-agnostic printf. Yeah!
 *
 * @author David Matlack
 */
#include <stdarg.h>
#include <fmt/doprnt.h>
#include <fmt/_printf.h>


static void _puts(struct printf_state *p, char *s) {
  while (*s) {
    p->putchar(*s);
    s++;
  }
  p->putchar('\n');
}

static void _flush(struct printf_state *p) {
	unsigned int i;

	for (i = 0; i < p->index; i++)
		p->putchar(p->buf[i]);

	p->index = 0;
}

static void _printf_callback(char *arg, int c) {
  struct printf_state *p = (struct printf_state *) arg;

	if (c == '\n') {
		p->buf[p->index] = 0;
		_puts(p, p->buf);
		p->index = 0;
	}
	else if ((c == 0) || (p->index >= _PRINTF_BUFMAX)) {
		_flush(p);
    p->putchar(c);
	}
	else {
		p->buf[p->index] = (char) c;
		p->index++;
	}
}

int _vprintf(struct printf_state *p, const char *fmt, va_list args) {

	p->index = 0;

	_doprnt(fmt, args, 0, (void (*)())_printf_callback, (char *) p);

	if (p->index != 0) {
    _flush(p);
  }

	return 0;
}
