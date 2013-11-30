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

#define SUCCEED_OR_DIE( function_call ) \
  do {\
    if (0 != function_call) {\
      panic("Failure in %s:%s:%d\n"\
            "    %s\n",\
             __FILE__, __func__, __LINE__, #function_call);\
    }\
  } while (0)

#endif /* __KERNEL_H__ */
