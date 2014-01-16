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

#include <boot/multiboot.h>
struct multiboot_info *__mb_info;

#include <fs/vfs.h>
static void vfs_test(char *path) {
  struct vfs_file *f;

  if (NULL != (f = vfs_get_file(path))) {
    INFO("Got vfs_file struct for %s\n"
         "          name: %s\n"
         "          inode: %u\n"
         "          perm: 0x%x\n"
         "          flags: 0x%x\n", 
         path, f->dirent->name, f->dirent->inode->inode,
         f->dirent->inode->perm, f->dirent->inode->flags);
    vfs_put_file(f);
  }
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

  /*
   * These should fail
   */
  vfs_test((char *)"foo/bar");
  vfs_test((char *)"/fail");
  vfs_test((char *)"/spin/foo");
  vfs_test((char *)"/not/a/legit/path");
  vfs_test((char *)"/spin/");

  /*
   * These should work
   */
  vfs_test((char *)"/");
  vfs_test((char *)"/spin");
  

  while (1) {
    irq_status_bar(VGA_ROWS - 1);
  }
}
