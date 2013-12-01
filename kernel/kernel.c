/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>
#include <dev/vga.h>
#include <dev/pit.h>
#include <mm/physmem.h>
#include <mm/vm.h>

#include <arch/x86/exn.h>
#include <arch/x86/pic.h>
#include <arch/x86/irq.h>
#include <arch/x86/idt.h>
#include <arch/x86/reg.h>

extern int __ticks;
extern int spurious_irqs;

void kernel_main() {

  SUCCEED_OR_DIE(log_init(dputchar, LOG_LEVEL_DEBUG));
  TRACE_ON;
  TRACE();

  LOG_PMEM_ZONE(PMEM_ZONE_KERNEL);
  LOG_PMEM_ZONE(PMEM_ZONE_USER);

  /* 
   * Initialize hardware interrupts by first telling the PIC where in the IDT it 
   * can find its interrupts handlers, and then installing the necessary interrupts 
   * handlers for each device connected to the PIC.
   * FIXME: move this machine dependent code elsewhere (interrupts_init?)
   */
  SUCCEED_OR_DIE(pic_init(IDT_PIC_MASTER_OFFSET, IDT_PIC_SLAVE_OFFSET));
  SUCCEED_OR_DIE(irq_init()); // call pic_init from in irq_init?

  SUCCEED_OR_DIE(vm_bootstrap());

  SUCCEED_OR_DIE(kmalloc_init());

  SUCCEED_OR_DIE(pmem_init());

  kprintf(
    "\n"
    "  All that is gold does not glitter,\n"
    "  Not all those who wander are lost;\n"
    "  The old that is strong does not wither,\n"
    "  Deep roots are not reached by the frost.\n"
    "\n"
    "  From the ashes a fire shall be woken,\n"
    "  A light from the shadows shall spring;\n"
    "  Renewed shall be blade that was broken,\n"
    "  The crownless again shall be king\n"
    "\n"
    "          J.R.R. Tolkien\n"
    "\n");

  SUCCEED_OR_DIE(pit_init(100));

  __enable_interrupts();

  while (__ticks < 1000) continue;
  __disable_interrupts();
  kprintf("number of spurious interrupts: %d\n", spurious_irqs);
}
