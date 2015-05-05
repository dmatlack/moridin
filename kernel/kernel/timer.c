/**
 * @file kernel/timer.c
 *
 */
#include <kernel/timer.h>
#include <kernel/config.h>
#include <kernel/sched.h>
#include <kernel/log.h>
#include <lib/assert.h>

/* The timer used to by the kernel. */
struct timer *timer = NULL;

void timer_tick(void)
{
	sched_tick();
}

void set_timer(struct timer *t)
{
	INFO("Setting kernel timer to %s.", t->name);
	timer = t;
}

void start_timer(int hz)
{
	ASSERT(timer);

	timer->start(timer, hz);
}
