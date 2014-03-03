/**
 * @file kernel/spinlock.c
 */
#include <kernel/spinlock.h>
#include <arch/atomic.h>
#include <arch/irq.h>

#include <assert.h>

static inline void __lock(struct spinlock *s, bool irq) {
  unsigned long my_ticket;

  /*
   * Disable interrupts _first_. We don't want to get our ticket, then
   * be interrupted or context switched out for a long period of time,
   * forcing all customers who tried to aquire the lock after us to spin
   * until we get back on the cpu.
   */
  if (irq) {
    s->irq = save_irqs();
  }

  my_ticket = atomic_add(&s->ticket, 1);

  /*
   * Spin until the ticket being served equals my ticket. Hopefully
   * gcc doesn't optimize any of this out...
   */
  while (my_ticket != s->serving) {
    if (irq) {
      panic("SMP is not supported... You should not be here!");
    }
    continue;
  }
}

static inline void __unlock(struct spinlock *s, bool irq) {
  atomic_add(&s->serving, 1);

  /*
   * We _must_ renable interrupts _after_ incrementing the ticket being
   * served. This is because we don't want to be context switched or
   * interrupted until after we make the spinlock available to the next
   * customer.
   */
  if (irq) {
    restore_irqs(s->irq);
  }
}

void spin_lock       (struct spinlock *s) { __lock(s, false);   }
void spin_lock_irq   (struct spinlock *s) { __lock(s, true);    }
void spin_unlock     (struct spinlock *s) { __unlock(s, false); }
void spin_unlock_irq (struct spinlock *s) { __unlock(s, true);  }
