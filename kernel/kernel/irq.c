/**
 * @file kernel/irq.c
 *
 * @brief Hardware Interrupts
 *
 */
#include <kernel/irq.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>
#include <mm/kmalloc.h>

#include <arch/atomic.h>

#include <kernel/debug.h>
#include <assert.h>
#include <list.h>
#include <errno.h>

#include <dev/vga.h>
#include <string.h>

list_typedef(struct irq_handler) irq_handler_list_t;
struct spinlock lock = INITIALIZED_SPINLOCK;

struct irq_desc {
	irq_handler_list_t handlers;
	int count;
} irq_descs[MAX_NUM_IRQS];

void irq_init(void)
{
	int i;

	for (i = 0; i < MAX_NUM_IRQS; i++) {
		struct irq_desc *desc = irq_descs + i;
		list_init(&desc->handlers);
		desc->count = 0;
	}
}

void kernel_irq_handler(int irq)
{
	struct irq_desc *desc = irq_descs + irq;
	struct irq_context context  = { .irq = irq };
	struct irq_handler *handler;
	unsigned long flags;

	spin_lock_irq(&lock, &flags);

	if (list_empty(&desc->handlers))
		goto out;

	atomic_add(&desc->count, 1);

	list_foreach(handler, &desc->handlers, link) {
		handler->f(&context);
	}

out:
	spin_unlock_irq(&lock, flags);
}

void irq_exit(void)
{
	maybe_reschedule();
}

int register_irq(int irq, struct irq_handler *new_handler)
{
	struct irq_desc *desc = irq_descs + irq;
	unsigned long flags;

	ASSERT_GREATEREQ(irq, 0);
	ASSERT_LESS(irq, MAX_NUM_IRQS);

	spin_lock_irq(&lock, &flags);

	list_insert_tail(&desc->handlers, new_handler, link);

	spin_unlock_irq(&lock, flags);

	return 0;
}
