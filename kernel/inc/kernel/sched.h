/**
 * @file kernel/sched.h
 */
#ifndef __KERNEL_SCHED_H__
#define __KERNEL_SCHED_H__

#include <kernel/proc.h>

void sched_init(void);
void make_runnable(struct thread *);
void sched_switch(void);
void sched_tick(void);
void reschedule(void);
void maybe_reschedule(void);
void child_return_from_fork(void);

extern void arch_sched_switch_end(void);

static inline void disable_save_preemption(void)
{
	struct thread *current = CURRENT_THREAD;

	current->preempt++;
}

static inline bool can_preempt(void)
{
	struct thread *current = CURRENT_THREAD;

	return (current->preempt == 0);
}

static inline void restore_preemption(void)
{
	struct thread *current = CURRENT_THREAD;

	if (--current->preempt)
		maybe_reschedule();
}

#endif /* !__KERNEL_SCHED_H__ */
