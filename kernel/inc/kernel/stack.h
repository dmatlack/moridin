/**
 * @file kernel/stack.h
 */
#ifndef __KERNEL_STACK_H__
#define __KERNEL_STACK_H__

#include <kernel/proc.h>

void setup_runtime_stack(struct thread_struct *thread, int argc, char **argv);
int create_user_stack(struct thread_struct *thread, int argc, char **argv);

#endif /* !__KERNEL_STACK_H__ */
