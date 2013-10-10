/**
 * @file mbentry.c
 *
 * @brief The multiboot entry point of the kernel.
 *
 * @author David Matlack
 */
#include <boot/multiboot.h>
#include <dev/vga.h>
#include <stddef.h>

void mb_entry(unsigned int mb_magic, struct multiboot_info mb_info) {

  /* 
   * Initialize the Video Graphics Array so we can start printing
   */
  if (vga_init()) {
    panic("Unable to initialize the VGA device.\n");
  }

  /*
   * Sut up a kernel printer using the VGA device
   */
  kputchar_set(vga_putbyte);

  /*
   * Check that we are in a multiboot environment.
   */
  if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    panic("Multiboot magic incorrect: expected 0x%08x, 0x%08x\n", mb_magic,
          MULTIBOOT_BOOTLOADER_MAGIC);
  }

  //FIXME remove
  kprintf("mb_magic = 0x%08x, mb_info = %p\n", mb_magic, mb_info);

  /*
   * And finally enter the kernel
   */
  kernel_main(0, NULL);
}
