/**
 * @file mbentry.c
 *
 * @brief The multiboot entry point of the kernel.
 *
 * @author David Matlack
 */
#include <boot/multiboot.h>

#include <kernel/kprintf.h>
#include <debug.h>
#include <dev/vga.h>
#include <stddef.h>
#include <x86/page.h>
#include <stdint.h>
#include <assert.h>

unsigned int num_phys_pages;

extern void kernel_main(int argc, char **argv);

/**
 * @brief The multiboot, C, entry-point to the kernel.
 */
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
    panic("Multiboot magic (eax) incorrect: expected 0x%08x, got 0x%08x\n",
          MULTIBOOT_BOOTLOADER_MAGIC, mb_magic);
  }

  mb_dump(dprintf, mb_info);

  /*
   * And finally enter the kernel
   */
  kernel_main(0, NULL);
}
