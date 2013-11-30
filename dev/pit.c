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

struct pic_device timer_device;

static void (*kernel_handler)(void);

static int freq_hz;

void pit_interrupt_handler(void) {
  if (NULL != kernel_handler) kernel_handler();
}

int pit_init(int hz, void (*handler)(void)) {
  int freq_div;

  kernel_handler = handler;

  timer_device.irq        = IRQ_TIMER;
  timer_device.handler    = pit_interrupt_handler;

  /*
   * Set up the timer to be fired at a specific interval
   */
  freq_hz = hz;
  (void)freq_div;
  //freq_div = PIT_FREQ_HZ / hz;
  //outb(PIT_COMMAND_PORT, IRQ_CHANNEL | LOHIBYTE | SQUARE_WAVE | BINARYMODE);
  //outb(PIT_CHANNEL0_PORT, freq_div && 0xff);        // low byte
  //outb(PIT_CHANNEL0_PORT, (freq_div >> 8) && 0xff); // high byte

  return 0;
}
