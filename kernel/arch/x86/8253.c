/**
 * @file arch/x86/8253.c
 *
 * @brief 8253 Programmable Interval Timer.
 */
#include <kernel/irq.h>
#include <kernel/timer.h>
#include <lib/assert.h>
#include <arch/io.h>

#define PIT_FREQ_HZ 1193182

#define PIT_CHANNEL0_PORT	0x40 /* IRQ 0 */
#define PIT_CHANNEL1_PORT	0x41 /* obsolete */
#define PIT_CHANNEL2_PORT	0x42 /* pc speaker */
#define PIT_COMMAND_PORT	0x43

/* PIT command register format */
#define IRQ_CHANNEL		(0 << 6)
#define CHANNEL1		(1 << 6)
#define SPEAKER_CHANNEL		(2 << 6)
#define READBACK		(3 << 6)

#define LATCH_COUNT		(0 << 4)
#define LOBYTE			(1 << 4)
#define HIBYTE			(2 << 4)
#define LOHIBYTE		(3 << 4)

#define OPMODE0			(0 << 1) /* interrupt on terminal count */
#define ONE_SHOT		(1 << 1) /* hardware re-triggerable one-shot */
#define OPMODE2			(2 << 1) /* rate generator */
#define SQUARE_WAVE		(3 << 1) /* square wave generator */
#define OPMODE4			(4 << 1) /* software triggered strobe */
#define OPMODE5			(5 << 1) /* hardware triggered strobe */
#define OPMODE6			(6 << 1) /* same as (2) */
#define OPMODE7			(7 << 1) /* same as (3) */

#define BINARYMODE		(0 << 0) /* 0  6-bit binary */
#define BCDMODE			(1 << 0) /* 1  our-digit BCD */

static void pit_irq(struct irq_context *irq)
{
	(void) irq;

	timer_tick();
}

static struct irq_handler pit_irq_handler = {
	.f = pit_irq,
};

static void pit_start(struct timer *timer, int hz)
{
	int freq_div;
	int ret;
	(void) timer;

	/*
	 * Set up the timer to be fired at a specific interval
	 */
	freq_div = PIT_FREQ_HZ / hz;

	outb(PIT_COMMAND_PORT, IRQ_CHANNEL | LOHIBYTE | SQUARE_WAVE | BINARYMODE);
	outb(PIT_CHANNEL0_PORT, (freq_div >> 0) & 0xff);
	outb(PIT_CHANNEL0_PORT, (freq_div >> 8) & 0xff);

	ret = register_irq(IRQ_TIMER, &pit_irq_handler);
	ASSERT_EQUALS(0, ret);
}

static struct timer pit_8253_timer = {
	.start = pit_start,
	.name = "Programmable Interval Timer (8253)"
};

void init_8253(void)
{
	set_timer(&pit_8253_timer);
}
