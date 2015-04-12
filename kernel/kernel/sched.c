/**
 * @file kernel/sched.c
 */
#include <kernel/sched.h>
#include <kernel/config.h>
#include <kernel/timer.h>
#include <arch/sched.h>
#include <arch/irq.h>
#include <arch/syscall.h>
#include <list.h>

static thread_list_t runnable;

static inline struct thread *runnable_dequeue(void)
{
	return list_dequeue(&runnable, sched_link);
}

static inline void runnable_enqueue(struct thread *thread)
{
	list_enqueue(&runnable, thread, sched_link);
}

void sched_make_runnable(struct thread *thread)
{
	list_enqueue(&runnable, thread, sched_link);
}

static inline bool runnable_empty(void)
{
	return list_empty(&runnable);
}

void sched_switch_begin(void)
{
	/*
	 * Prevent scheduler ticks from trying to schedule us while
	 * we're already scheduling!
	 */
	disable_irqs();
}

void sched_switch_end(void)
{
	arch_sched_switch_end();

	enable_irqs();
}

void reschedule(void)
{
	clear_flags(RESCHEDULE);
	sched_switch();
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
	start_timer(CONFIG_TIMER_HZ);
}

void sched_tick(void)
{
	set_flags(RESCHEDULE);
}
