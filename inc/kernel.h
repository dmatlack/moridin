/**
 * @file kernel.h
 *
 * @brief All stardard kernel functions and includes.
 *
 * @author David Matlack
 */
#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Formatted printing to the console.
 */
int kprintf(const char *fmt, ...);

#endif /* __KERNEL_H__ */
