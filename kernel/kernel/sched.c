/**
 * @file kernel/sched.c
 */
#include <kernel/sched.h>
#include <kernel/config.h>
#include <kernel/timer.h>
#include <arch/sched.h>
#include <arch/irq.h>
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

void sched_switch(void)
{
	struct thread *next;

	runnable_enqueue(CURRENT_THREAD);
	next = runnable_dequeue();

	context_switch(next);
}

void sched_init(void)
{
	start_timer(CONFIG_TIMER_HZ);
}
