/**
 * @file kernel/sched.h
 */
#ifndef __KERNEL_SCHED_H__
#define __KERNEL_SCHED_H__

#include <kernel/proc.h>

void sched_make_runnable(struct thread *);
void sched_switch(void);
void sched_init(void);
void sched_tick(void);

/* Schedule out the current thread and run a different thread. */
void reschedule(void);

void child_return_from_fork(void);

extern void arch_sched_switch_end(void);

#endif /* !__KERNEL_SCHED_H__ */
