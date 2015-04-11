/**
 * @file kernel/timer.c
 *
 */
#include <kernel/timer.h>
#include <kernel/irq.h>
#include <kernel/config.h>

#include <dev/pit.h>

#include <kernel/debug.h>
#include <errno.h>

void timer_irq(struct irq_context *irq)
{
	(void) irq;
}

struct irq_handler __timer_handler = {
	.f = timer_irq,
};

int __timer_hz;


/**
 * @brief Initialize the system timer. This is the timer used for multitasking.
 */
void timer_init(void)
{
	int hz = CONFIG_TIMER_HZ;
	int ret;

	TRACE("");

	/*
	 * Initialize the timer hardware
	 */
	ret = pit_init(hz);
	ASSERT(!ret);

	/*
	 * Register a handler for timer interrupts
	 */
	ret = register_irq(IRQ_TIMER, &__timer_handler);
	ASSERT(!ret);
}
