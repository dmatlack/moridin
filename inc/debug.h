/**
 * @file debug.h
 *
 * @brief Debugging utilities for the kernel.
 *
 * @author David Matlack
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

/**
 * @brief Formatted printing for debugging purposes.
 *
 * To change where dprintf prints characters, change the putchar method
 * being used in debug/dprintf.c.
 */
int dprintf(const char *fmt, ...);

#endif /* __DEBUG_H__ */
