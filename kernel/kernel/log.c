/**
 * @file kernel/log.c
 *
 * TODO: support time-stamped log messages
 */
#include <kernel/debug.h>

#include <stddef.h>
#include <types.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include <kernel/kprintf.h>

struct logger {
	struct printf_state state;
	int level;
};

static struct logger logger;

void log_init(int (*putchar)(int), int level)
{
	memset(&logger, 0, sizeof(logger));

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
