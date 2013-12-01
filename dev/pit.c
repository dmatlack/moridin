/**
 * @file dev/pit.c
 *
 * @brief Programmable Interval Timer (PIT)
 * 
 * @author David Matlack
 */
#include <dev/pit.h>

#include <arch/x86/pic.h>
#include <arch/x86/irq.h>
#include <arch/x86/io.h>
#include <kernel/kprintf.h>
#include <stddef.h>
#include <debug.h>

static int freq_hz;

static struct irq_handler pit_irq_handler;

int __ticks = 0;

void tick(struct irq_context *context) {
  (void) context;
  __ticks++;
}

int pit_init(int hz) {
  int freq_div;

  __ticks = 0;

  pit_irq_handler.top_handler    = tick;
  pit_irq_handler.bottom_handler = NULL;
  
  register_irq(IRQ_TIMER, &pit_irq_handler);

  /*
   * Set up the timer to be fired at a specific interval
   */
  freq_hz = hz;
  (void)freq_div;
  freq_div = PIT_FREQ_HZ / hz;
  INFO("freq_div=%d\n", freq_div);

  outb(PIT_COMMAND_PORT, IRQ_CHANNEL | LOHIBYTE | SQUARE_WAVE | BINARYMODE);
  outb(PIT_CHANNEL0_PORT, (freq_div >> 0) & 0xff); // low byte
  outb(PIT_CHANNEL0_PORT, (freq_div >> 8) & 0xff); // high byte

  return 0;
}
