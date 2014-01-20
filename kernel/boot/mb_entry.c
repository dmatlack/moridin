/**
 * @file mbentry.c
 *
 * @brief The multiboot entry point of the kernel.
 *
 * @author David Matlack
 */
#include <boot/multiboot.h>

#include <kernel/kprintf.h>
#include <kernel/kmalloc.h>

#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>

#include <dev/vga.h>

#include <mm/memory.h>

#include <fs/initrd.h>

#include <arch/x86/cpu.h>
#include <arch/x86/reg.h>
#include <arch/x86/exn.h>

extern void kernel_main(void);

struct multiboot_info *__mb_info;

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
  int ret;

  __mb_info = mb_info;

  vga_init();
  vga_set_color(VGA_WHITE);
  kputchar_set(vga_putbyte);

  if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    panic("Multiboot magic (eax) incorrect: expected 0x%08x, got 0x%08x\n",
          MULTIBOOT_BOOTLOADER_MAGIC, mb_magic);
  }

  /*
   * Use the mb_info struct to learn about the physical memory layout
   */
  mem_mb_init(mb_info);

  kprintf("boot stack:    0x%08x, 0x%08x\n", boot_stack_bottom, boot_stack_top);
  kprintf("boot page_dir: 0x%08x\n", boot_page_dir);
  kprintf("kernel image:  0x%08x, 0x%08x\n", (size_t) kimg_start, (size_t) kimg_end);
  kprintf("kernel heap:   0x%08x, 0x%08x\n", kheap_start, kheap_end);

  /*
   * Set up kmalloc to only allocate dynamic memory in the first 16 MB of
   * memory. This will allow us to use kmalloc during early startup.
   *
   * NOTE: if we use a higher half kernel we'll have to offset these
   * values
   */
  kmalloc_early_init((size_t) kheap_start, (size_t) kheap_end);

  /*
   * Initialize the ramdisk
   *
   * ASSUMPTION: initrd is the 0th module loaded by GRUB
   */
  if ((ret = initrd_init(mb_mod_start(mb_info, 0)))) {
    panic("Failed to initialize the initial ramdisk: %s (%s)",
          ret, strerr(ret));
  }

  /* 
   * Initialize the exception handlers in the IDT incase an exception occurs
   * during system setup.
   */
  x86_exn_setup_idt();

  /*
   * Disable the floating point unit
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
