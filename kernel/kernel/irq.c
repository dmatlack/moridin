/**
 * @file kernel/irq.c
 *
 * @brief Hardware Interrupts
 *
 */
#include <kernel/irq.h>
#include <kernel/sched.h>
#include <mm/kmalloc.h>

#include <arch/atomic.h>

#include <kernel/debug.h>
#include <assert.h>
#include <list.h>
#include <errno.h>

#include <dev/vga.h>
#include <string.h>

list_typedef(struct irq_handler) irq_handler_list_t;

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

	if (list_empty(&desc->handlers))
		return;

	atomic_add(&desc->count, 1);

	list_foreach(handler, &desc->handlers, link) {
		handler->f(&context);
	}
}

void irq_exit(void)
{
	maybe_reschedule();
}

int register_irq(int irq, struct irq_handler *new_handler)
{
	struct irq_desc *desc = irq_descs + irq;

	ASSERT_GREATEREQ(irq, 0);
	ASSERT_LESS(irq, MAX_NUM_IRQS);

	list_insert_tail(&desc->handlers, new_handler, link);

	return 0;
}

/**
 * @brief Print bar of text accross a row of the console with some useful
 * information about IRQs.
 *
 * @param bar_row The row to draw the bar.
 */
void irq_status_bar(int bar_row)
{
	int old_row, old_col;
	char old_color;
	int i;

	vga_get_cursor(&old_row, &old_col);
	old_color = vga_get_color();

	vga_set_color(VGA_COLOR(VGA_WHITE, VGA_BLUE));

	vga_set_cursor(bar_row, 0);
	kprintf("IRQs: ");
	for (i = 0; i < MAX_NUM_IRQS; i++) {
		if (irq_descs[i].count > 0)
			kprintf("%d:%d ", i, irq_descs[i].count);
	}

	vga_set_cursor(old_row, old_col);
	vga_set_color(old_color);
}
