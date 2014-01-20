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
  char buf[512];
  ssize_t bytes;
  int i;

  (void) buf; (void) bytes;
  f = vfs_get_file((char*) "/hello.txt");
  if (NULL == f) {
    kprintf("Failed to open /hello.txt\n");
    return;
  }
  do {
    bytes = vfs_read(f, buf, 512);
    for (i = 0; i < bytes; i++) kputchar(buf[i]);
  } while (bytes == 512);
  kputchar('\n');
  vfs_put_file(f);

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

  while (1) {
    irq_status_bar(VGA_ROWS - 1);
  }
}
