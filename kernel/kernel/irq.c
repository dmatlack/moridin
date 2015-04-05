/**
 * @file kernel/irq.c
 *
 * @brief Hardware Interrupts
 *
 */
#include <kernel/irq.h>
#include <mm/kmalloc.h>

#include <arch/atomic.h>

#include <kernel/debug.h>
#include <assert.h>
#include <list.h>
#include <errno.h>

#include <dev/vga.h>
#include <string.h>

struct irq_state *irq_states;

void irq_init(void)
{
	int i;

	TRACE();

	irq_states = kmalloc(sizeof(struct irq_state) * MAX_NUM_IRQS);
	if (NULL == irq_states) {
		panic("Not enough memory to allocate the irq list.");
	}

	for (i = 0; i < MAX_NUM_IRQS; i++) {
		struct irq_state *irq = irq_states + i;
		list_init(&irq->handlers);
		irq->count = 0;
		irq->in_irq = 0;
	}
}

void kernel_irq_handler(int irq)
{
	struct irq_context context;
	struct irq_state *state;
	struct irq_handler *handler;
	int prev_in_irq;

	state = &irq_states[irq];
	atomic_add(&state->count, 1);

	prev_in_irq = atomic_add(&state->in_irq, 1);
	ASSERT_EQUALS(prev_in_irq, 0);

	context.irq = irq;
	list_foreach(handler, &state->handlers, link) {
		if (handler->top_handler) handler->top_handler(&context);
	}

	atomic_add(&state->in_irq, -1);

	ack_irq(irq);
}

void register_irq(int irq, struct irq_handler *new_handler)
{
	ASSERT_GREATEREQ(irq, 0);
	ASSERT_LESS(irq, MAX_NUM_IRQS);

	list_elem_init(new_handler, link);
	list_insert_tail(&irq_states[irq].handlers, new_handler, link);
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
		if (irq_states[i].count > 0)
			kprintf("%d:%d ", i, irq_states[i].count);
	}

	vga_set_cursor(old_row, old_col);
	vga_set_color(old_color);
}
