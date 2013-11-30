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
#include <kernel/kprintf.h>
#include <stddef.h>

struct pic_device timer_device;

static void (*kernel_handler)(void);
static int ticks;

void pit_interrupt_handler(void) {
  ticks++;
  kprintf("ticks: %d\n", ticks);

  if (NULL != kernel_handler) kernel_handler();

  //outb(EOI) or something like that

  //TODO allow the kernel to register a top and bottom half handler?
}

int pit_init(void (*handler)(void)) {
  kernel_handler = handler;

  timer_device.irq        = IRQ_TIMER;
  timer_device.handler    = pit_interrupt_handler;

  if (0 != pic_register_device(&timer_device)) {
    return -1;
  }

  //TODO program the actual timer ... :)

  return 0;
}
