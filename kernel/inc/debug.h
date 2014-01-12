/**
 * @file debug.h
 *
 * @brief Debugging utilities for the kernel.
 *
 * @author David Matlack
 */
#include <debug/log.h>
#include <debug/bochs.h>

#ifndef __DEBUG_H__
#define __DEBUG_H__

void debug_init(void);
int debug_putchar(int c);

#endif /* !__DEBUG_H__ */
