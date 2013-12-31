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
#include <arch/x86/page.h>
#include <stdint.h>
#include <assert.h>
#include <mm/physmem.h>
#include <arch/x86/cpu.h>
#include <arch/x86/reg.h>
#include <arch/x86/exn.h>

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

  if (vga_init()) {
    panic("Unable to initialize the VGA device.\n");
  }
  kputchar_set(vga_putbyte);

  if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    panic("Multiboot magic (eax) incorrect: expected 0x%08x, got 0x%08x\n",
          MULTIBOOT_BOOTLOADER_MAGIC, mb_magic);
  }

  mb_dump(dprintf, mb_info);

  /* 
   * initialize the x86 exception handling facilities and install the kernel's
   * exception handler
   */
  if (x86_exn_init(mb_exn_handler)) {
    panic("Unable to install x86 exception handlers.\n");
  }

  if (pmem_bootstrap(MB(1) + (1024 * mb_info->mem_upper), X86_PAGE_SIZE,
                     __kernel_image_start, __kernel_image_end)) {
    panic("Unable to initialize memory constructs\n");
  }

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
