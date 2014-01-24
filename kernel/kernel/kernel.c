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
#include <mm/vm.h>

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
  char *init = (char *) "/init";
  int i;

  f = vfs_get_file(init);
  if (NULL == f) {
    kprintf("Failed to open %s\n", init);
    return;
  }
  if ((i = load(f)) < 0) {
    kprintf("Failed to load %s: %s", init, strerr(i));
  }
  vfs_put_file(f);
}

void kernel_main() {

  serial_port_init();
  debug_init();
  log_init(debug_putchar, LOG_LEVEL_DEBUG);

  mb_dump(__log, __mb_info);
  
  pages_init();
  vm_init();
  exn_init();
  irq_init();
  timer_init();

  enable_irqs();
  
  pci_init();

  /*
   * Temporary stuff...
   */
  vfs_test();
  while (1) {
    irq_status_bar(VGA_ROWS - 1);
  }
}
