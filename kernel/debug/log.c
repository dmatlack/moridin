/**
 * @file debug/log.c
 *
 * @author David Matlack
 */
#include <debug.h>

#include <stddef.h>
#include <types.h>
#include <stdarg.h>
#include <errno.h>

#include <kernel/kprintf.h>

struct logger __logger;

int log_putchar(int c) {
  if (__logger.flags & LOG_TO_SERIAL_PORT) {
    serial_putchar(__logger.serial, (char) c);
  }
  
  if (__logger.provided_puchar) {
    __logger.provided_puchar(c);
  }

  return c;
}

/**
 * @brief Initialize the logging system.
 *
 * @param putchar The putchar method to use for printing. Can be NULL if
 *                you want to use LOG_TO_SERIAL_PORT.
 *
 * @param level   The verbosity level of logging to use. This can always
 *                be changed dynamically with log_setlevel().
 *
 * @param flags   Optional features, such as logging to serial ports.
 *
 * @return
 *    ENODEV if serial port logging is request but there aren't any ports
 *           available for use
 *
 *    0      otherwise
 */
int log_init(int (*putchar)(int), int level, int flags) {
  __logger.pstate.putchar = log_putchar;
  __logger.provided_puchar = putchar;

  __logger.level = level;
  __logger.trace_on = 0;
  __logger.flags = flags;

  if (flags & LOG_TO_SERIAL_PORT) {
    __logger.serial = request_serial_port("kernel-logging");
    if (NULL == __logger.serial) {
      flags &= ~LOG_TO_SERIAL_PORT;
      return ENODEV;
    }
  }

  return 0;
}

void log_setputchar(int (*putchar)(int)) {
  __logger.provided_puchar = putchar;
}

void log_setlevel(int level) {
  __logger.level = level;
}


static void __log_putstring(const char *s) {
  while ((char)0 != *s) {
    log_putchar((int) *s);
    s++;
  }
}


int log(int log_level, const char *prefix,  const char *fmt, ...) {
  va_list args;
  int err;

  if (log_level <= __logger.level) {
    __log_putstring(prefix);
    va_start(args, fmt);
    err = _vprintf(&__logger.pstate, fmt, args);
    va_end(args);
  }

  return err;
}

int trace(const char *fmt, ...) {
  va_list args;
  int err;

  if (__logger.trace_on > 0) {
    va_start(args, fmt);
    err = _vprintf(&__logger.pstate, fmt, args);
    va_end(args);
  }

  return err;
}
