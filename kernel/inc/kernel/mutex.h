#ifndef __KERNEL_MUTEX_H__
#define __KERNEL_MUTEX_H__

#include <kernel/spinlock.h>
#include <kernel/proc.h>

#include <lib/list.h>

struct mutex {
	struct spinlock lock;
	thread_list_t threads; /* uses mutex_link */
	struct thread *owner;
};

#define INITIALIZED_MUTEX {						\
	.lock = INITIALIZED_SPINLOCK,					\
	.threads = INITIALIZED_EMPTY_LIST,				\
	.owner = NULL,							\
}

static inline void mutex_init(struct mutex *m)
{
	spin_lock_init(&m->lock);
	list_init(&m->threads);
	m->owner = NULL;
}

/* blocks the calling thread until the mutex is aquired */
void mutex_aquire(struct mutex *m);

/* releases the mutex and awakens any threads blocked on the mutex */
void mutex_release(struct mutex *m);

#endif /* !__KERNEL_MUTEX_H__ */
