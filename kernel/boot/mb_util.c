/**
 * @file mb_util.c
 *
 * @brief Utility functions for the multiboot start up code.
 *
 */
#include <boot/multiboot.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <kernel/kprintf.h>

size_t mb_mod_start(struct multiboot_info *mb_info, int index) {
  return (size_t)
    (((multiboot_module_t *) mb_info->mods_addr) + index)->mod_start;
}

/**
 * @brief Dump the contents of the multiboot_info struct using the given
 * printf function.
 *
 * @param p The printf function to use to do printing.
 * @param mb_info The address of the multiboot info struct.
 */
void mb_dump(printf_f p, struct multiboot_info *mb_info) {
  uint32_t flags = mb_info->flags;

  p("struct multiboot_info *: %p\n\n", mb_info);
  p("spec: http://www.gnu.org/software/grub/manual/multiboot/multiboot.txt\n");

#define IF_FLAGS(_flag) \
  p("%s: %d\n", #_flag, (flags & (_flag)) ? 1 : 0); \
  if (flags & (_flag)) 

  IF_FLAGS( MULTIBOOT_INFO_MEMORY ) {
    p("  mem_lower = 0x%08x\n", mb_info->mem_lower * 1024);
    p("  mem_upper = 0x%08x\n", mb_info->mem_upper * 1024);
  }

  IF_FLAGS( MULTIBOOT_INFO_BOOTDEV ) {
    p("  boot_device = 0x%08x\n", mb_info->boot_device);
  }

  IF_FLAGS( MULTIBOOT_INFO_CMDLINE ) {
    p("  cmd_line = %s\n", (char *) mb_info->cmdline);
  }

  IF_FLAGS( MULTIBOOT_INFO_MODS ) {
    unsigned i;

    p("  mods_count = %d\n", mb_info->mods_count);
    p("  mods_addr = 0x%08x\n", mb_info->mods_addr);

    for (i = 0; i < mb_info->mods_count; i++) {
      multiboot_module_t *mod;

      mod = ((multiboot_module_t *) mb_info->mods_addr) + i;
      p("    %d: start=0x%08x, end=0x%08x, size=0x%x, cmdline=%s\n",
          i, mod->mod_start, mod->mod_end, mod->mod_end - mod->mod_start,
          (char *) mod->cmdline);
    }
  }

  IF_FLAGS( MULTIBOOT_INFO_AOUT_SYMS ) {
    p("  aout_sym:\n");
    p("    tabsize = 0x%08x\n", mb_info->u.aout_sym.tabsize); 
    p("    strsize = 0x%08x\n", mb_info->u.aout_sym.strsize); 
    p("    addr = 0x%08x\n", mb_info->u.aout_sym.addr); 
    p("    reserved = 0x%08x\n", mb_info->u.aout_sym.reserved); 
  }

  IF_FLAGS( MULTIBOOT_INFO_ELF_SHDR ) {
    p("  elf_sec:\n");
    p("    num = 0x%08x\n", mb_info->u.elf_sec.num); 
    p("    size = 0x%08x\n", mb_info->u.elf_sec.size); 
    p("    addr = 0x%08x\n", mb_info->u.elf_sec.addr); 
    p("    shndx = 0x%08x\n", mb_info->u.elf_sec.shndx); 
  }

  IF_FLAGS( MULTIBOOT_INFO_MEM_MAP ) {
    struct multiboot_mmap_entry *e;

    p("  mmap_length = 0x%08x\n", mb_info->mmap_length);
    p("  mmap_addr = 0x%08x\n", mb_info->mmap_addr);

    e = (struct multiboot_mmap_entry *) mb_info->mmap_addr;
    while ((unsigned int) e < (mb_info->mmap_addr + mb_info->mmap_length)) {
      p("    start = 0x%08llx, end = 0x%08llx, len = 0x%08llx, type = %d (%s)\n",
          e->addr, e->addr + e->len, e->len, e->type,
          e->type == MULTIBOOT_MEMORY_AVAILABLE ? "available" : "reserved");
      e++;
    }
  }

  IF_FLAGS( MULTIBOOT_INFO_DRIVE_INFO ) {
    p("  (not implemented)\n");
  }

  IF_FLAGS( MULTIBOOT_INFO_CONFIG_TABLE ) {
    p("  (not implemented)\n");
  }

  IF_FLAGS( MULTIBOOT_INFO_BOOT_LOADER_NAME ) {
    p("  boot_loader_name = %s\n", mb_info->boot_loader_name);
  }

  IF_FLAGS( MULTIBOOT_INFO_APM_TABLE ) {
    p("  (not implemented)\n");
  }
  
  IF_FLAGS( MULTIBOOT_INFO_VIDEO_INFO ) {
    p("  (not implemented)\n");
  }

#undef FLAGS
}
