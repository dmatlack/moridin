/**
 * @file kernel/spinlock.c
 */
#include <kernel/spinlock.h>
#include <kernel/sched.h>
#include <kernel/proc.h>
#include <arch/atomic.h>
#include <arch/irq.h>

#include <assert.h>

/* TODO memory barriers */

static inline void __lock(struct spinlock *s)
{
	int my_ticket;

	// FIXME: spinlock rely on overflow to correctly work
	my_ticket = atomic_add(&s->ticket, 1);

	/*
	 * Spin until the ticket being served equals my ticket. Hopefully
	 * gcc doesn't optimize any of this out...
	 */
	while (my_ticket != s->serving) {
		panic("SMP is not supported... You should not be here!");
		continue;
	}
}

static inline void __unlock(struct spinlock *s)
{
	s->serving++;
}

void __spin_lock(struct spinlock *s)
{
	__lock(s);
}

void __spin_unlock(struct spinlock *s)
{
	__unlock(s);
}

void __spin_lock_irq(struct spinlock *s, unsigned long *flags)
{
	disable_save_irqs(flags);
	__spin_lock(s);
}

void __spin_unlock_irq(struct spinlock *s, unsigned long flags)
{
	__spin_unlock(s);
	restore_irqs(flags);
}

void spin_lock(struct spinlock *s)
{
	disable_save_preemption();
	__lock(s);
}

void spin_unlock(struct spinlock *s)
{
	__unlock(s);
	restore_preemption();
}

void spin_lock_irq(struct spinlock *s, unsigned long *flags)
{
	disable_save_irqs(flags);
	spin_lock(s);
}

void spin_unlock_irq(struct spinlock *s, unsigned long flags)
{
	spin_unlock(s);
	restore_irqs(flags);
}
