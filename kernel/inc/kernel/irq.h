/**
 * @file kernel/irq.h
 *
 */
#ifndef __KERNEL_IRQ_H__
#define __KERNEL_IRQ_H__

#include <lib/list.h>
#include <arch/irq.h>

struct irq_context {
	int irq;
};

typedef void (*irq_handler_f)(struct irq_context *context);

struct irq_handler {
	irq_handler_f f;
	list_link(struct irq_handler) link;
};

void irq_init(void);
int register_irq(int irq, struct irq_handler *new_handler);
void kernel_irq_handler(int irq);

/* called at the very end of all interrupt handlers. */
void irq_exit(void);

void irq_status_bar(int bar_row);

#endif /* !__KERNEL_IRQ_H__ */
