/**
 * @file dev/pit.c
 *
 * @brief Programmable Interval Timer (PIT)
 * 
 * @author David Matlack
 */
#include <dev/pit.h>

#include <kernel/io.h>
#include <kernel/kprintf.h>
#include <stddef.h>
#include <debug.h>

static int freq_hz;

int pit_init(int hz) {
  int freq_div;

  /*
   * Set up the timer to be fired at a specific interval
   */
  freq_hz = hz;
  freq_div = PIT_FREQ_HZ / hz;

  outb(PIT_COMMAND_PORT, IRQ_CHANNEL | LOHIBYTE | SQUARE_WAVE | BINARYMODE);
  outb(PIT_CHANNEL0_PORT, (freq_div >> 0) & 0xff); // low byte
  outb(PIT_CHANNEL0_PORT, (freq_div >> 8) & 0xff); // high byte

  return 0;
}
