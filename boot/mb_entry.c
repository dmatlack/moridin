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

void mb_parse(struct multiboot_info *mb_info) {
  uint32_t flags = mb_info->flags;
  printf_f p = dprintf;

  p("struct multiboot_info *: %p\n\n", mb_info);

#define F(n) ((flags >> (n)) & 0x1)
#define IF_FLAGS(n) \
  p("flags[%d] = %d\n", n, F(n)); \
  if (F(n)) 

  IF_FLAGS(0) {
    p("  mem_upper = 0x%08x KB\n", mb_info->mem_upper);
    p("  mem_lower = 0x%08x KB\n", mb_info->mem_lower);
  }

  IF_FLAGS(1) {
    p("  boot_device = 0x%08x\n", mb_info->boot_device);
  }

  IF_FLAGS(2) {
    p("  cmd_line = %s\n", (char *) mb_info->cmdline);
  }

  IF_FLAGS(3) {
    p("  mods_count = %d\n", mb_info->mods_count);
    p("  mods_addr = 0x%08x\n", mb_info->mods_addr);
  }

  IF_FLAGS(4) {
    p("  aout_sym:\n");
    p("    tabsize = 0x%08x\n", mb_info->u.aout_sym.tabsize); 
    p("    strsize = 0x%08x\n", mb_info->u.aout_sym.strsize); 
    p("    addr = 0x%08x\n", mb_info->u.aout_sym.addr); 
    p("    reserved = 0x%08x\n", mb_info->u.aout_sym.reserved); 
  }

  IF_FLAGS(5) {
    p("  elf_sec:\n");
    p("    num = 0x%08x\n", mb_info->u.elf_sec.num); 
    p("    size = 0x%08x\n", mb_info->u.elf_sec.size); 
    p("    addr = 0x%08x\n", mb_info->u.elf_sec.addr); 
    p("    shndx = 0x%08x\n", mb_info->u.elf_sec.shndx); 
  }

  IF_FLAGS(6) {
    p("  mmap_length = 0x%08x\n", mb_info->mmap_length);
    p("  mmap_addr = 0x%08x\n", mb_info->mmap_addr);
  }

  IF_FLAGS(7) {
    p("  TODO\n");
  }

  IF_FLAGS(8) {
    p("  TODO\n");
  }

  IF_FLAGS(9) {
    p("  boot_loader_name = %s\n", mb_info->boot_loader_name);
  }

  IF_FLAGS(10) {
    p("  TODO\n");
  }
  
  IF_FLAGS(11) {
    p("  TODO\n");
  }

#undef FLAGS
}

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

  mb_parse(mb_info);

  /*
   * And finally enter the kernel
   */
  kernel_main(0, NULL);
}
