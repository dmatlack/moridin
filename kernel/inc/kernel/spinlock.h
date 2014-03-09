/**
 * @file kernel/spinlock.h
 */
#ifndef __KERNEL_SPINLOCK_H__
#define __KERNEL_SPINLOCK_H__

struct spinlock {
  int ticket;              /* the next ticket to hand out */
  volatile int serving;    /* the ticket of the customer currently being served */
  int irq;                 /* interrupt state */
};

#define SPINLOCK_INIT(_s) \
  do {                    \
    (_s)->ticket     = 0; \
    (_s)->serving    = 0; \
    (_s)->irq        = 0; \
  } while (0)

void spin_lock       (struct spinlock *s);
void spin_lock_irq   (struct spinlock *s);
void spin_unlock     (struct spinlock *s);
void spin_unlock_irq (struct spinlock *s);

#endif /* !__KERNEL_SPINLOCK_H__ */
