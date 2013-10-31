/**
 * @file debug/log.h
 */
#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__

#include <fmt/_printf.h>
#include <types.h>
#include <stddef.h>

#define LOG_LEVEL_OFF  -1
#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN  1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

struct logger {
  struct printf_state pstate;
  int level;
  int trace_on;
};

int log_init(int (*putchar)(int), int level);
void log_setputchar(int (*putchar)(int));
void log_setlevel(int level);

//TODO support more powerful prefixes (like PIDs)
int log(int log_level, const char *prefix, const char *fmt, ...);
int trace(const char *prefix, const char *fmt, ...);

#define TRACE_ON      do { __log.trace_on++; } while (0)
#define TRACE_RESTORE do { __log.trace_on--; } while (0)
#define TRACE_OFF     do { __log.trace_on = 0; } while (0)

#define WARN(fmt, ...) log(LOG_LEVEL_WARN, "[WARN] ", fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) log(LOG_LEVEL_INFO, "[INFO] ", fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) log(LOG_LEVEL_ERROR, "[ERROR] ", fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) log(LOG_LEVEL_DEBUG, "[DEBUG] ", fmt, ##__VA_ARGS__)
#define TRACE(fmt, ...) trace(fmt, "[TRACE] ", __VA_ARGS__)

#endif /* !__DEBUG_LOG_H__ */
