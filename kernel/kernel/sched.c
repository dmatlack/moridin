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
#include <assert.h>

struct scheduler {
	thread_list_t runnable;
	struct spinlock lock;
};

struct scheduler scheduler;

/*
 * FIXME: make_runnable does not work on SMP because the thread could
 * already be running on another cpu.
 */
void make_runnable(struct thread *thread)
{
	struct scheduler *s = &scheduler;
	unsigned long flags;

	spin_lock_irq(&s->lock, &flags);

	ASSERT_NOTEQUALS(thread, CURRENT_THREAD);
	ASSERT_NOTEQUALS(thread->state, EXITED);
	thread->state = RUNNABLE;

	list_enqueue(&s->runnable, thread, state_link);

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

	INFO("Context Switch to %d:%d.", current->proc->pid, current->tid);

	arch_sched_switch_end();

	__spin_unlock_irq(&s->lock, current->sched_switch_irqs);
}

void reschedule(void)
{
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

	if (current->state == RUNNABLE)
		list_enqueue(&s->runnable, current, state_link);

	next = list_dequeue(&s->runnable, state_link);
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
