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
#include <arch/x86/idt.h>

extern void kernel_main(void);

extern char kernel_idt[];
extern char kernel_gdt[];
extern char kernel_tss[];

struct multiboot_info *__mb_info;

extern void x86_syscall(void);

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

  ASSERT_EQUALS((size_t) kernel_idt, 0x10000c);
  ASSERT_EQUALS((size_t) kernel_gdt, 0x10080c);
  ASSERT_EQUALS((size_t) kernel_tss, 0x10083c);

  ASSERT_EQUALS(mb_magic, MULTIBOOT_BOOTLOADER_MAGIC);

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
  kmalloc_early_init((size_t) kheap_start,
                    ((size_t) kheap_end - (size_t) kheap_start));

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
   * Initialize the Interrupt Descriptor Table (IDT), installing default
   * handlers for all entries and exception handlers for exceptions.
   */
  x86_exn_setup_idt();

  /*
   * Disable the floating point unit
   */
  x86_disable_fpu();

  /*
   * Set up the system call handler
   */
  idt_syscall_gate(0x80, x86_syscall);

  /*
   * And finally enter the kernel
   *
   * TODO: GRUB/multiboot supports passing in a command line to the kernel
   * (argv, envp). If we want to support that we should parse the cmdline
   * field of the multiboot info struct and pass it into the kernel.
   */
  kernel_main();
}
