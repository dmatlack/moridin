/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <assert.h>
#include <debug.h>

#include <kernel/kmalloc.h>
#include <kernel/irq.h>
#include <kernel/timer.h>

#include <mm/vm.h>
#include <mm/physmem.h>

#include <dev/vga.h>
#include <dev/serial.h>
#include <dev/pci.h>

extern int __max_irqs;
extern struct irq_state *__irqs;

void kernel_main() {
  /*
   * Serial Port (needed for debug logging)
   */
  SUCCEED_OR_DIE(serial_init());

  /*
   * Debugging
   */
  SUCCEED_OR_DIE(debug_init());

  /*
   * Logging
   */
  SUCCEED_OR_DIE(log_init(debug_putchar, LOG_LEVEL_DEBUG));

  /*
   * Virtual Memory Bootstrap
   */
  SUCCEED_OR_DIE(vm_bootstrap());

  /*
   * Kernel Dynamic Memory Allocation
   */
  SUCCEED_OR_DIE(kmalloc_init());

  /*
   * Hardware Interrupts
   */
  SUCCEED_OR_DIE(irq_init());

  /*
   * Physical Memory Manager
   */
  SUCCEED_OR_DIE(pmem_init());

  /*
   * Peripheral Component Interconnect
   */
  SUCCEED_OR_DIE(pci_init());

  /*
   * Kernel Timer
   */
  SUCCEED_OR_DIE(timer_init());

  enable_irqs();

  vga_set_color(VGA_COLOR(VGA_WHITE, VGA_BLUE));
  vga_set_cursor(24, 0);
  kprintf("                                                                               "); 
  while (1) {
    // FIXME
    //  Kernel Crash if you uncomment out the following lines!!!
#if 0
    int i;
    vga_set_cursor(24, 0);
    kprintf("IRQ: ");
    for (i = 0; i < __max_irqs; i++) {
      int count;
     
      disable_irqs();
      count = __irqs[i].count;
      if (count > 0) {
        kprintf("%d: %d ", i, count);
      }
      enable_irqs();
    }
#endif
  }
}
