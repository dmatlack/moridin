/**
 * @file kernel/stack.h
 */
#ifndef __KERNEL_STACK_H__
#define __KERNEL_STACK_H__

#include <kernel/proc.h>

int create_process_stack(int argc, char **argv);

#endif /* !__KERNEL_STACK_H__ */
