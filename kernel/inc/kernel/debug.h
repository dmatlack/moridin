/**
 * @file debug.h
 *
 * @brief Debugging utilities for the kernel.
 *
 */
#include <kernel/log.h>
#include <dev/bochs.h>
#include <assert.h>

#ifndef __DEBUG_H__
#define __DEBUG_H__

void debug_init(void);
int debug_putchar(int c);

#endif /* !__DEBUG_H__ */
