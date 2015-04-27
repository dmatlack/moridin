/**
 * @file kernel/wait.c.
 */
#include <kernel/spinlock.h>
#include <kernel/wait.h>
#include <kernel/proc.h>
#include <kernel/sched.h>

void begin_wait(struct wait *wait)
{
	struct thread *current = CURRENT_THREAD;
	unsigned long flags;

	spin_lock_irq(&wait->lock, &flags);

	list_enqueue(&wait->threads, current, state_link);
	current->state = BLOCKED;

	spin_unlock_irq(&wait->lock, flags);
}

void kick(struct wait *wait)
{
	unsigned long flags;

	spin_lock_irq(&wait->lock, &flags);

	while (!list_empty(&wait->threads))
		make_runnable(list_dequeue(&wait->threads, state_link));

	spin_unlock_irq(&wait->lock, flags);
}
