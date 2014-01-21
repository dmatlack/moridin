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
#include <errno.h>

#include <boot/multiboot.h>
struct multiboot_info *__mb_info;

#include <fs/vfs.h>
extern int load(struct vfs_file *);

void vfs_test(void) {
  struct vfs_file *f;
  int i;

  f = vfs_get_file((char*) "/spin");
  if (NULL == f) {
    kprintf("Failed to open /spin\n");
    return;
  }
  if ((i = load(f)) < 0) {
    kprintf("Failed to load /spin: %s", strerr(i));
  }
  vfs_put_file(f);
}

#include <arch/x86/vm.h>
void x86_vm_test(void) {
  size_t *ppages;
  size_t *vpages;
  int ret;
  int num_pages = 10;
  int i;
 
  ppages = kmalloc(sizeof(size_t) * num_pages);
  ASSERT_NOT_NULL(ppages);
  vpages = kmalloc(sizeof(size_t) * num_pages);
  ASSERT_NOT_NULL(vpages);

  for (i = 0; i < num_pages; i++) {
    vpages[i] = 0xC0000000 + (PAGE_SIZE * i);
  }

  ret = alloc_pages(num_pages, ppages);
  ASSERT_EQUALS(ret, 0);

  x86_map_pages((struct entry_table *) boot_page_dir, vpages, ppages, num_pages, VM_R|VM_W|VM_U);

  // Use pages

  x86_unmap_pages((struct entry_table *) boot_page_dir, vpages, ppages, num_pages);

  free_pages(num_pages, ppages);

  kfree(ppages, sizeof(size_t) * num_pages);
  kfree(vpages, sizeof(size_t) * num_pages);
}

void kernel_main() {

  serial_port_init();
  debug_init();
  log_init(debug_putchar, LOG_LEVEL_DEBUG);

  mb_dump(__log, __mb_info);
  
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

  vfs_test();

  x86_vm_test();

  while (1) {
    irq_status_bar(VGA_ROWS - 1);
  }
}
