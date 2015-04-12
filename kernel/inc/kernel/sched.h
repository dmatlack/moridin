/**
 * @file kernel/sched.h
 */
#ifndef __KERNEL_SCHED_H__
#define __KERNEL_SCHED_H__

#include <kernel/proc.h>

void sched_make_runnable(struct thread *);
void sched_switch(void);
void sched_init(void);

#endif /* !__KERNEL_SCHED_H__ */
