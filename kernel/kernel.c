/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>
#include <dev/vga.h>
#include <arch/x86/exn.h>
#include <arch/x86/pic.h>
#include <mm/physmem.h>
#include <mm/vm.h>

#include <arch/x86/irq.h>
#include <arch/x86/idt.h>

static int __ticks = 0;

void tick(void) {
  __ticks++;
}

extern void __generate_irq(uint8_t irq);

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

  /*
   * Look at our fancy irq routines...
   */
  idt_irq_gate(IRQ_TIMER, tick);

  generate_irq(IRQ_TIMER); // 1
  generate_irq(IRQ_TIMER); // 2
  generate_irq(IRQ_TIMER); // 3
  generate_irq(IRQ_TIMER); // 4
  generate_irq(IRQ_TIMER); // 5

  kprintf("num ticks: %d\n", __ticks);


  /* 
   * it's ok to return from kernel. it will get us back to boot/boot.S where 
   * we just twidle our thumbs
   */
}
