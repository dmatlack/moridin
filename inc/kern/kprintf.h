/**
 * @file kprintf.h
 *
 * @author David Matlack
 */
#ifndef __KPRINTF_H__
#define __KPRINTF_H__

#include <stdarg.h>

int kputchar(int c);
int kputs(const char *s);
int kvprintf(const char *fmt, va_list args);
int kprintf(const char *fmt, ...)
           __attribute__((__format__ (__kprintf__, 1, 2)));

#endif /* __KPRINTF_H__ */
