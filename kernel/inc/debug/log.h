/**
 * @file debug/log.h
 */
#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__

#include <fmt/_printf.h>
#include <types.h>
#include <stddef.h>
#include <dev/serial.h>

#define LOG_LEVEL_OFF  -1
#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN  1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

struct logger {
  struct printf_state pstate;
  int (*provided_puchar)(int);

  int level;
  int trace_on;

  int flags;
    #define LOG_TO_SERIAL_PORT (1 << 0)

  struct serial_port *serial;
};


extern struct logger __logger;

int log_init(int (*putchar)(int), int level, int flags);
void log_setputchar(int (*putchar)(int));
void log_setlevel(int level);

//TODO support more powerful prefixes (like PIDs)
int log(int log_level, const char *prefix, const char *fmt, ...);
int trace(const char *fmt, ...);

#define TRACE_ON      do { __logger.trace_on++; } while (0)
#define TRACE_RESTORE do { __logger.trace_on--; } while (0)
#define TRACE_OFF     do { __logger.trace_on = 0; } while (0)

#define INFO(fmt, ...) log(LOG_LEVEL_INFO, "[INFO]   ", fmt"\n", ##__VA_ARGS__)
#define WARN(fmt, ...) log(LOG_LEVEL_WARN, "[WARN]   ", fmt"\n", ##__VA_ARGS__)
#define ERROR(fmt, ...) log(LOG_LEVEL_ERROR, "[ERROR]  ", fmt"\n", ##__VA_ARGS__)

#ifdef KDEBUG
    #define DEBUG(fmt, ...) log(LOG_LEVEL_DEBUG, "[DEBUG]  ", fmt"\n", ##__VA_ARGS__)
    #define TRACE(fmt, ...) trace("[TRACE]  %s("fmt")\n", __func__, ##__VA_ARGS__)
#else
    #define DEBUG(fmt, ...)
    #define TRACE(fmt, ...)
#endif

#endif /* !__DEBUG_LOG_H__ */
