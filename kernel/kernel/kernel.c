/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <kernel/kmalloc.h>
#include <kernel/irq.h>
#include <kernel/timer.h>
#include <kernel/exn.h>

#include <mm/memory.h>
#include <mm/pages.h>

#include <dev/vga.h>
#include <dev/serial.h>
#include <dev/pci.h>

#include <assert.h>
#include <debug.h>

void kernel_main() {

  serial_port_init();
  debug_init();
  log_init(debug_putchar, LOG_LEVEL_DEBUG);
  
  /*
   * Set up kmalloc to only allocate dynamic memory in the first 16 MB of
   * memory. This will allow us to use kmalloc during early startup.
   *
   * NOTE: if we use a higher half kernel we'll have to offset these
   * values
   */
  kmalloc_early_init((size_t) kimg_end, MB(16));
  
  /*
   * Page management
   */
  SUCCEED_OR_DIE(pages_init());

  /*
   * Exception Handling
   */
  SUCCEED_OR_DIE(exn_init());

  /*
   * Hardware Interrupts
   */
  SUCCEED_OR_DIE(irq_init());

  /*
   * Kernel Timer
   */
  SUCCEED_OR_DIE(timer_init());

  enable_irqs();
  
  /*
   * Peripheral Component Interconnect
   */
  SUCCEED_OR_DIE(pci_init());

  while (1) {
    irq_status_bar(VGA_ROWS - 1);
  }
}
