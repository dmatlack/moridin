#include <kernel/mutex.h>
#include <kernel/proc.h>
#include <kernel/sched.h>

void mutex_aquire(struct mutex *m)
{
	struct thread *current = CURRENT_THREAD;
	unsigned long flags;

	spin_lock_irq(&m->lock, &flags);

	while (m->owner) {
		list_enqueue(&m->threads, current, state_link);
		current->state = BLOCKED;

		disable_save_preemption();
		spin_unlock_irq(&m->lock, flags);

		/*
		 * I think racing with the owner releasing the mutex is
		 * ok here. We will just reschedule when we really could
		 * have kept running.
		 *
		 * This assumes make_runnable() (which sets our state to
		 * runnable) cannot run concurrently with reschedule()
		 * (which makes a decides whether to add us back on the
		 * runqueue based on our state). Currently, that assumpt-
		 * ion holds.
		 */
		reschedule();

		spin_lock_irq(&m->lock, &flags);
		restore_preemption();
	}

	m->owner = current;
	spin_unlock_irq(&m->lock, flags);
}

void mutex_release(struct mutex *m)
{
	unsigned long flags;

	spin_lock_irq(&m->lock, &flags);

	while (!list_empty(&m->threads)) {
		struct thread *thread = list_dequeue(&m->threads, state_link);
		make_runnable(thread);
	}

	m->owner = NULL;
	spin_unlock_irq(&m->lock, flags);
}
