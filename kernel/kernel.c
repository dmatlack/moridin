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

  log_init(dputchar, LOG_LEVEL_DEBUG);

  INFO("Welcome to kernel_main()!\n");

  /* 
   * Initialize hardware interrupts by first telling the PIC where in the IDT it 
   * can find its interrupts handlers, and then installing the necessary interrupts 
   * handlers for each device connected to the PIC.
   * FIXME: move this machine dependent code elsewhere (interrupts_init?)
   */
  if (pic_init(IDT_PIC_MASTER_OFFSET, IDT_PIC_SLAVE_OFFSET)) {
    panic("Unable to initialize the PIC.\n");
  }

  vm_bootstrap();

  kmalloc_init();

  pmem_map_dump(dprintf);
  dprintf("VM_ZONE_KERNEL:\n");
  dprintf("   address: 0x%08x\n", VM_ZONE_KERNEL->address);
  dprintf("   size:    0x%08x\n", VM_ZONE_KERNEL->size);
  dprintf("VM_ZONE_USER:\n");
  dprintf("   address: 0x%08x\n", VM_ZONE_USER->address);
  dprintf("   size:    0x%08x\n", VM_ZONE_USER->size);

  pmem_init();

  {
    int i;

    assert(0 == pmem_alloc(debug_pages, 10, PMEM_ZONE_USER));
    dprintf("Alloced 10 pages:\n");
    for (i = 0; i < 10; i++) {
      dprintf("    0x%08x\n", debug_pages[i]);
    }
    assert(0 == pmem_alloc(debug_pages, 10, PMEM_ZONE_USER));
    dprintf("Alloced 10 pages:\n");
    for (i = 0; i < 10; i++) {
      dprintf("    0x%08x\n", debug_pages[i]);
    }

    pmem_free(debug_pages, 10, PMEM_ZONE_USER);
  }

  while (1) continue;
}
