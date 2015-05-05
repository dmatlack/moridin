/**
 * @file kernel/log.c
 *
 * TODO: support time-stamped log messages
 */
#include <kernel/log.h>
#include <kernel/config.h>
#include <dev/bochs.h>
#include <dev/serial.h>

#include <stddef.h>
#include <types.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

struct logger {
	struct printf_state state;
	int level;
	struct serial_port *serial_port;
};

static struct logger logger;

int log_putchar(int c)
{
	if (logger.serial_port)
		serial_putchar(logger.serial_port, (char) c);

#ifdef BOCHS
	bochs_putchar(c);
#endif

	return c;
}

void log_init(void)
{
	logger.level = CONFIG_LOG_LEVEL;
	logger.state.putchar = log_putchar;

	/* TODO: allow other ways of logging. */
	logger.serial_port = reserve_serial_port("log");
	ASSERT(logger.serial_port);
}

void early_log_init(int (*putchar)(int), int level)
{
	logger.level = level;
	logger.state.putchar = putchar;
}

bool log_check(int level)
{
	return level <= logger.level;
}

int log(const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);

	ret = _vprintf(&logger.state, fmt, args);

	va_end(args);

	return ret;
}
