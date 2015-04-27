#ifndef __KERNEL_WAIT_H__
#define __KERNEL_WAIT_H__

#include <kernel/spinlock.h>
#include <kernel/proc_types.h>
#include <lib/list.h>

struct wait {
	struct spinlock lock;
	thread_list_t threads;
};

#define INITIALIZED_WAIT {						\
	.lock = INITIALIZED_SPINLOCK,					\
	.threads = INITIALIZED_EMPTY_LIST,				\
}

static inline void wait_init(struct wait *wait)
{
	spin_lock_init(&wait->lock);
	list_init(&wait->threads);
}

/*
 * begin_wait adds the calling thread onto a queue of waiting threads
 * defined by the wait struct and sets the thread's state to BLOCKED.
 * It is intended that begin_wait is called shortly before calling
 * reschedule.
 *
 * begin_wait() is inherently racy with resepect to kick() because
 * adding a thread to the wait queue and rescheduling that thread is
 * not an atomic operation.
 *
 * Case 1:
 *   Thread A     Thread B
 *   begin_wait
 *   reschedule
 *                kick
 *
 *   This is the expected case. When Thread A calls reschedule its
 *   state will be BLOCKED and it will not be added back onto the
 *   runqueue (so it will not run). The call to kick will take it
 *   off the wait queue and put it onto the runqueue.
 *
 * Case 2:
 *   Thread A     Thread B
 *   begin_wait
 *                kick
 *   reschedule
 *
 *   In this case Thread A will reschedule not with a BLOCKED state
 *   but with a runnable state. The scheduler will place it back onto
 *   the run queue and it will run sometime soon.
 *
 * kick() and reschedule() may race but the scheduler will make sure
 * reschedule() doesn't race with making the thread runnable.
 */
void begin_wait(struct wait *wait);

/*
 * Wake up all threads on the wait queue.
 */
void kick(struct wait *wait);

#endif /* !__KERNEL_WAIT_H__ */
