/**
 * @file kernel.c
 *
 * Welcome. Have a look around.
 *
 */
#include <kernel/config.h>
#include <kernel/debug.h>
#include <kernel/init.h>
#include <kernel/irq.h>
#include <mm/kmalloc.h>
#include <kernel/loader.h>
#include <kernel/proc.h>
#include <kernel/stack.h>
#include <kernel/timer.h>

#include <mm/memory.h>
#include <mm/pages.h>
#include <mm/vm.h>

#include <dev/vga.h>
#include <dev/serial.h>
#include <dev/pci.h>

#include <assert.h>
#include <errno.h>

#include <fs/initrd.h>
#include <fs/vfs.h>

/**
 * @brief This is main logical entry point for the kernel, not to be confused
 * with the actual entry point, _start. Also not to be confused the multiboot
 * entry point mb_entry. Ok so it's not the entry point, but it is an entry
 * point.
 */
void kernel_main() {
  debug_init();

  /*
   * Set up kmalloc to only allocate dynamic memory in the first 16 MB of
   * memory. This will allow us to use kmalloc during early startup.
   *
   * NOTE: if we use a higher half kernel we'll have to offset these
   * values
   */
  kmalloc_early_init((size_t) kheap_start,
                    ((size_t) kheap_end - (size_t) kheap_start));
  pages_init();
  vm_init();
  irq_init();
  timer_init();
  initrd_init();

  enable_irqs();

  pci_init();

  /////////////////////////////////////////////////////////////////////////////
  // Temporary hack to get to userspace with some test argv/arc
  /////////////////////////////////////////////////////////////////////////////
  {
    char *argv[4] = { "/init", "arg1", "arg2", ":)" };
    int argc = 4;

    // TODO: pass in the name of the init binary as a parameter via the
    // bootloader rather than hardcopying it.
    run_init((char *) "/init", argc, argv );
  }
}
