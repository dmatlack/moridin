/**
 * @file kernel/timer.c
 *
 * @author David Matlack
 */
#include <kernel/timer.h>
#include <kernel/irq.h>

#include <dev/pit.h>

#include <debug.h>

struct irq_handler timer_handler;

/**
 * @brief Initialize the system timer. This is the timer used for
 * multitasking.
 * @param hz The frequency at which to receive interrupts from the timer.
 */
int timer_init(int hz) {

  /*
   * Initialize the hardware
   */
  if (0 != pit_init(hz)) {
    return -1;
  }

  /*
   * Register a handler for timer interrupts
   */
  timer_handler.top_handler    = NULL;
  timer_handler.bottom_handler = NULL;
  register_irq(0 /*FIXME*/, &timer_handler);

  return 0;
}
