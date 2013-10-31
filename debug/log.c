/**
 * @file debug/log.c
 *
 * @author David Matlack
 */
#include <debug.h>

#include <stddef.h>
#include <types.h>
#include <stdarg.h>

struct logger __log;

int log_init(int (*putchar)(int), int level) {
  __log.pstate.putchar = putchar;
  __log.level = level;
  __log.trace_on = 0;
  return 0;
}

void log_setputchar(int (*putchar)(int)) {
  __log.pstate.putchar = putchar;
}

void log_setlevel(int level) {
  __log.level = level;
}

static void __log_putstring(const char *s) {
  while ((char)0 != *s) {
    __log.pstate.putchar((int) *s);
    s++;
  }
}


int log(int log_level, const char *prefix,  const char *fmt, ...) {
  va_list args;
  int err;

  if (log_level <= __log.level) {
    __log_putstring(prefix);
    va_start(args, fmt);
    err = _vprintf(&__log.pstate, fmt, args);
    va_end(args);
  }

  return err;
}

int trace(const char *prefix, const char *fmt, ...) {
  va_list args;
  int err;

  if (__log.trace_on > 0) {
    __log_putstring(prefix);
    va_start(args, fmt);
    err = _vprintf(&__log.pstate, fmt, args);
    va_end(args);
  }

  return err;
}
