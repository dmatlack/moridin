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
	thread_list_t exited;
	struct spinlock lock;
};

struct scheduler scheduler;

static inline struct thread *dequeue(thread_list_t *list)
{
	return list_dequeue(list, sched_link);
}

static inline void enqueue(thread_list_t *list, struct thread *thread)
{
	list_enqueue(list, thread, sched_link);
}

void make_runnable(struct thread *thread)
{
	struct scheduler *s = &scheduler;
	unsigned long flags;

	spin_lock_irq(&s->lock, &flags);

	enqueue(&s->runnable, thread);

	spin_unlock_irq(&s->lock, flags);
}

void sched_switch_begin(void)
{
	struct thread *current = CURRENT_THREAD;
	struct scheduler *s = &scheduler;

	/*
	 * Use the __spin_lock variation in order to avoid the preemption
	 * handling code in the spin lock implementation.
	 */
	__spin_lock_irq(&s->lock, &current->sched_switch_irqs);
}

void sched_switch_end(void)
{
	struct thread *current = CURRENT_THREAD;
	struct scheduler *s = &scheduler;

	arch_sched_switch_end();

	__spin_unlock_irq(&s->lock, current->sched_switch_irqs);

	INFO("Context Switch to %d:%d.", current->proc->pid, current->tid);
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
	struct scheduler *s = &scheduler;
	struct thread *current = CURRENT_THREAD;
	struct thread *next;

	sched_switch_begin();

	if (check_state(RUNNABLE))
		enqueue(&s->runnable, current);
	if (check_state(EXITED))
		enqueue(&s->exited, current);

	next = dequeue(&s->runnable);
	ASSERT(next);

	if (next == current)
		goto out;

	context_switch(next);

	/*
	 * Think carefully before adding code between context_switch and
	 * sched_switch_end.
	 */
out:
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
