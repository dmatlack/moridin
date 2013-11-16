/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>

#include <dev/vga.h>

#include <x86/exn.h>
#include <x86/pic.h>

#include <mm/physmem.h>
#include <mm/vm.h>

#include <kernel/proc.h>

void *debug_pages[10];

void kernel_main() {

  SUCCEED_OR_DIE(log_init(dputchar, LOG_LEVEL_DEBUG));
  TRACE_ON;
  TRACE();

  LOG_PMEM_ZONE(PMEM_ZONE_KERNEL);
  LOG_PMEM_ZONE(PMEM_ZONE_USER);

  {
    size_t size = KERNEL_IMAGE_END - KERNEL_IMAGE_START;
    INFO("Kernel Image: start=0x%08x, end=0x0x%08x, size=0x%08x (%d MB, %d KB)",
       KERNEL_IMAGE_START, KERNEL_IMAGE_END, size, size / MB(1), size % MB(1) / KB(1));
  }

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

  kprintf("huzzah! :)\n");
  while (1) continue;
}
