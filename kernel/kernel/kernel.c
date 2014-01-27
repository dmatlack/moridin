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
#include <kernel/loader.h>

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

extern struct vm_space *postboot_vm_space;

void load_test(struct vfs_file *f, struct vm_space *space) {
  int i;
  if ((i = load(f, space)) < 0) {
    kprintf("Failed to load %s: %s", f->dirent->name, strerr(i));
  }
}

void vfs_test(void) {
  struct vfs_file *f;

  f = vfs_get_file((char *) "/init");
  if (NULL == f) {
    kprintf("Failed to open %s\n", "/init");
    return;
  }

  kprintf("Bytes before: 0x%x\n", kmalloc_bytes_used());
  load_test(f, postboot_vm_space);
  kprintf("Bytes after: 0x%x\n", kmalloc_bytes_used());

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
