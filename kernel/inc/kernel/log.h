/**
 * @file debug/log.h
 */
#ifndef __KERNEL_LOG_H__
#define __KERNEL_LOG_H__

#include <kernel/config.h>
#include <fmt/_printf.h>
#include <types.h>
#include <stddef.h>

/*
 * Set up logging during early boot (will not use any locks, memory
 * allocations, or any other features which require a full kernel
 * initialization.
 */
void early_log_init(int (*putchar)(int), int level);

void log_init(int (*putchar)(int), int level);
int log(const char *fmt, ...);
bool log_check(int level);

#define log_level(_level, _fmt, ...) do {				\
	if (!log_check(_level))						\
		break;							\
	log(_fmt, ##__VA_ARGS__);					\
} while (0)

#define INFO(_fmt, ...)	  log_level(LOG_INFO, "I "_fmt"\n", ##__VA_ARGS__)
#define WARN(_fmt, ...)   log_level(LOG_WARN, "W "_fmt"\n", ##__VA_ARGS__)
#define ERROR(_fmt, ...)  log_level(LOG_ERROR, "E "_fmt"\n", ##__VA_ARGS__)
#define DEBUG(_fmt, ...)  log_level(LOG_DEBUG, "D "_fmt"\n", ##__VA_ARGS__)
#define FATAL(_fmt, ...)  log("F "_fmt"\n", ##__VA_ARGS__)

#define TRACE(_fmt, ...) DEBUG("%s("_fmt")", __func__, ##__VA_ARGS__)

#endif /* !__KERNEL_LOG_H__ */
