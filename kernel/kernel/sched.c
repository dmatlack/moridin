/**
 * @file kernel/sched.c
 */
#include <kernel/sched.h>
#include <kernel/spinlock.h>
#include <kernel/config.h>
#include <kernel/timer.h>
#include <arch/sched.h>
#include <arch/irq.h>
#include <arch/syscall.h>
#include <list.h>

struct scheduler {
	thread_list_t runnable;
	struct spinlock lock;
	unsigned long irq_flags;
};

struct scheduler scheduler;

static inline struct thread *runnable_dequeue(void)
{
	struct scheduler *s = &scheduler;

	return list_dequeue(&s->runnable, sched_link);
}

static inline void runnable_enqueue(struct thread *thread)
{
	struct scheduler *s = &scheduler;

	list_enqueue(&s->runnable, thread, sched_link);
}

void make_runnable(struct thread *thread)
{
	struct scheduler *s = &scheduler;
	unsigned long flags;

	spin_lock_irq(&s->lock, &flags);

	runnable_enqueue(thread);

	spin_unlock_irq(&s->lock, flags);
}

static inline bool runnable_empty(void)
{
	struct scheduler *s = &scheduler;

	return list_empty(&s->runnable);
}

void sched_switch_begin(void)
{
	struct scheduler *s = &scheduler;

	/*
	 * Use the __spin_lock variation in order to avoid the preemption
	 * handling code in the spin lock implementation.
	 */
	__spin_lock_irq(&s->lock, &s->irq_flags);
}

void sched_switch_end(void)
{
	struct scheduler *s = &scheduler;

	arch_sched_switch_end();

	__spin_unlock_irq(&s->lock, s->irq_flags);
}

void reschedule(void)
{
	ASSERT(can_preempt());

	clear_flags(RESCHEDULE);
	sched_switch();
}

void maybe_reschedule(void)
{
	if (!check_flags(RESCHEDULE))
		return;

	if (!can_preempt())
		return;

	reschedule();
}

void sched_switch(void)
{
	struct thread *current = CURRENT_THREAD;
	struct thread *next;

	sched_switch_begin();

	if (runnable_empty())
		goto end;

	runnable_enqueue(current);
	next = runnable_dequeue();

	context_switch(next);

	/*
	 * Think carefully before adding code between context_switch and
	 * sched_switch_end.
	 */
end:
	sched_switch_end();
}

void child_return_from_fork(void)
{
	sched_switch_end();
	return_from_syscall(0);
}

void sched_init(void)
{
	struct scheduler *s = &scheduler;

	list_init(&s->runnable);
	spin_lock_init(&s->lock);

	start_timer(CONFIG_TIMER_HZ);
}

void sched_tick(void)
{
	set_flags(RESCHEDULE);
}
