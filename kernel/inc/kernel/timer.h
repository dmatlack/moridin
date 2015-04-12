/**
 * @file kernel/timer.h
 */
#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

struct timer {
	/* Schedule interrupt at the provided frequency and start the timer. */
	void (*start)(struct timer *timer, int hz);
	const char *name;
};

void set_timer(struct timer *t);
void start_timer(int hz);
void timer_tick(void);

#endif /* !__KERNEL_TIMER_H__ */
