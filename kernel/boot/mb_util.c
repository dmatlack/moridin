/**
 * @file mb_util.c
 *
 * @brief Utility functions for the multiboot start up code.
 *
 * @author David Matlack
 */
#include <boot/multiboot.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <kernel/kprintf.h>
#include <arch/x86/exn.h>

void mb_exn_handler(struct x86_exn_args *args) {
  struct x86_iret_stack *iret = &args->iret;
  struct x86_pusha_stack *pusha = &args->pusha;
  struct x86_exn *exn = &x86_exceptions[args->vector];

  kprintf("\n"
  "-------------------------------------------------------------------\n"
  "%d %s %s (cause: %s)\n"
  "-------------------------------------------------------------------\n"
  "eip: 0x%08x\n"
  "esp: 0x%08x\n"
  "ebp: 0x%08x\n"
  "\n"
  "edi: 0x%08x esi: 0x%08x\n"
  "eax: 0x%08x ebx: 0x%08x\n"
  "ecx: 0x%08x edx: 0x%08x\n"
  "\n"
  "cr0: 0x%08x\n"
  "cr2: 0x%08x\n"
  "cr3: 0x%08x\n"
  "cr4: 0x%08x\n"
  "\n"
  "ds: 0x%08x\n"
  "es: 0x%08x\n"
  "fs: 0x%08x\n"
  "gs: 0x%08x\n"
  "\n"
  "error: %d\n"
  "-------------------------------------------------------------------\n",
    exn->vector, exn->mnemonic, exn->description, exn->cause,
    iret->eip,
    iret->esp,
    pusha->ebp,
    pusha->edi, pusha->esi,
    pusha->eax, pusha->ebx,
    pusha->ecx, pusha->edx,
    args->cr0,
    args->cr2,
    args->cr3,
    args->cr4,
    args->ds,
    args->es,
    args->fs,
    args->gs,
    args->error_code
  );

  panic("Exception %d during boot. Aborting.", exn->vector);
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

  ASSERT(p == kprintf);
  
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
    p("  mods_count = %d\n", mb_info->mods_count);
    p("  mods_addr = 0x%08x\n", mb_info->mods_addr);
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
    p("  TODO\n");
  }

  IF_FLAGS( MULTIBOOT_INFO_CONFIG_TABLE ) {
    p("  TODO\n");
  }

  IF_FLAGS( MULTIBOOT_INFO_BOOT_LOADER_NAME ) {
    p("  boot_loader_name = %s\n", mb_info->boot_loader_name);
  }

  IF_FLAGS( MULTIBOOT_INFO_APM_TABLE ) {
    p("  TODO\n");
  }
  
  IF_FLAGS( MULTIBOOT_INFO_VIDEO_INFO ) {
    p("  TODO\n");
  }

#undef FLAGS
}
