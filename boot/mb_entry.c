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
#include <mm/mem.h>
#include <x86/cpu.h>
#include <x86/reg.h>


unsigned int num_phys_pages;

extern char __kernel_image_start[];
extern char __kernel_image_end[];

extern void kernel_main(void);

/**
 * @brief The multiboot, C, entry-point to the kernel.
 *
 * This function should set up anything the kernel needs to run that is
 * dependent on this being a multiboot environment (and thus the presence
 * of the multiboot_info struct).
 *
 * @param mb_magic eax, magic value that confirms we are in multiboot
 * @param mb_info ebx, the multiboot_info struct 
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

  /*
   * read in information about the kernel image
   */
  kernel_image_start = (size_t) __kernel_image_start;
  kernel_image_end = (size_t) __kernel_image_end;

  /*
   * determine where the kernel memory ends, and user memory begins
   */
  user_mem_start = 512 * MEGABYTE; //FIXME how to choose the size of user memory
  user_mem_end = MEGABYTE + mb_info->mem_upper * KILOBYTE;
  assert(user_mem_end > user_mem_start);

  /*
   * Initialize the kernel's dynamic memory manager.
   */
  if (mb_lmm_init(mb_info)) {
    panic("Unable to initialize kernel dynamic memory.\n");
  }

  /*
   * disable the floating point unit for now
   */
  x86_disable_fpu();

  /*
   * And finally enter the kernel
   *
   * TODO: GRUB/multiboot supports passing in a command line to the kernel
   * (argv, envp). If we want to support that we should parse the cmdline
   * field of the multiboot info struct and pass it into the kernel.
   */
  kernel_main();
}
