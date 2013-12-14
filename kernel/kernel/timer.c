/**
 * @file kernel/timer.c
 *
 * @author David Matlack
 */
#include <kernel/timer.h>
#include <kernel/irq.h>
#include <kernel/config.h>

#include <dev/pit.h>

#include <debug.h>

struct irq_handler __timer_handler;
int __timer_hz;


/**
 * @brief Initialize the system timer. This is the timer used for multitasking.
 */
int timer_init(void) {
  int ret;

  TRACE("");

  __timer_hz = CONFIG_TIMER_HZ;

  /*
   * Initialize the timer hardware
   */
  if (0 != (ret = pit_init(__timer_hz))) {
    return ret;
  }

  /*
   * Register a handler for timer interrupts
   */
  __timer_handler.top_handler    = NULL;
  __timer_handler.bottom_handler = NULL;

  register_irq(0, &__timer_handler);

  return 0;
}
