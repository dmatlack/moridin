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
  TRACE("void");

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

  while (1) continue;
}
