/**
 * @file kernel/spinlock.h
 */
#ifndef __KERNEL_SPINLOCK_H__
#define __KERNEL_SPINLOCK_H__

struct spinlock {
	/* the next ticket to hand out */
	int ticket;
	/* the ticket of the customer currently being served */
	int serving;
};

#define INITIALIZED_SPINLOCK = {					\
	.ticket = 0,							\
	.serving = 0,							\
}

static inline void spin_lock_init(struct spinlock *s)
{
	s->ticket = 0;
	s->serving = 0;
}

/*
 * Does nothing except aquire and release a spin lock.
 *
 * Reasons to use this instead of spin_{lock,unlock}:
 *   - You don't want preemption disabled during the lock.
 *   - You don't want to potentially reschedule on unlock.
 */
void __spin_lock(struct spinlock *s);
void __spin_unlock(struct spinlock *s);
void __spin_lock_irq(struct spinlock *s, unsigned long *flags);
void __spin_unlock_irq(struct spinlock *s, unsigned long flags);

void spin_lock(struct spinlock *s);
void spin_unlock(struct spinlock *s);
void spin_lock_irq(struct spinlock *s, unsigned long *flags);
void spin_unlock_irq(struct spinlock *s, unsigned long flags);

#endif /* !__KERNEL_SPINLOCK_H__ */
