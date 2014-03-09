/**
 * @file kernel/sched.h
 */
#ifndef __KERNEL_SCHED_H__
#define __KERNEL_SCHED_H__

#include <kernel/proc.h>

void schedule(struct thread *);

#endif /* !__KERNEL_SCHED_H__ */
