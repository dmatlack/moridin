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
#include <kernel/exn.h>

#include <mm/vm.h>
#include <mm/physmem.h>

#include <dev/vga.h>
#include <dev/serial.h>
#include <dev/pci.h>

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
   * Exception Handling
   */
  SUCCEED_OR_DIE(exn_init());

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
