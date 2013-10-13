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

#include <mm/lmm.h>
extern lmm_t kernel_lmm;
//TODO kmalloc

#include <x86/exn.h>
void kernel_x86_exn_handler(struct x86_exn_args *exn);

#endif /* __KERNEL_H__ */
