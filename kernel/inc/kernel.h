/**
 * @file kernel.h
 *
 * @brief All stardard kernel functions and includes.
 *
 * @author David Matlack
 */
#ifndef __KERNEL_H__
#define __KERNEL_H__

/*
 * Standard includes things will need
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <kernel/kprintf.h>

#include <kernel/kmalloc.h>

#include <arch/x86/exn.h>
void kernel_x86_exn_handler(struct x86_exn_args *exn);

void __succeed_or_die(int expr, const char * exprstr,
                      const char *file, const char *func, int line);
#define SUCCEED_OR_DIE( _expr ) \
  do { \
    __succeed_or_die(_expr, #_expr, __FILE__, __func__, __LINE__); \
  } while (0)

void __succeed_or_warn(int expr, const char * exprstr,
                       const char *file, const char *func, int line);
#define SUCCEED_OR_WARN( _expr ) \
  do { \
    __succeed_or_warn(_expr, #_expr, __FILE__, __func__, __LINE__); \
  } while (0)

#endif /* __KERNEL_H__ */
