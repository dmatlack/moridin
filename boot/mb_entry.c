/**
 * @file mbentry.c
 *
 * @brief The multiboot entry point of the kernel.
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <boot/multiboot.h>
#include <dev/vga.h>
#include <stddef.h>
#include <x86/page.h>

unsigned int num_phys_pages;

void mb_entry(unsigned int mb_magic, struct multiboot_info *mb_info) {

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
    panic("Multiboot magic (eax) incorrect: expected 0x%08x, 0x%08x\n", 
          mb_magic, MULTIBOOT_BOOTLOADER_MAGIC);
  }

  //FIXME overflow?
  num_phys_pages = (mb_info->mem_upper * 1024) / PAGE_SIZE;

  /*
   * And finally enter the kernel
   */
  kernel_main(0, NULL);
}
