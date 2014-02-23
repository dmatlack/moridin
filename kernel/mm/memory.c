/**
 * @file mm/memory.c
 */
#include <mm/memory.h>
#include <boot/multiboot.h>
#include <stddef.h>
#include <assert.h>
#include <kernel.h>

size_t phys_mem_bytes; /* size of physical memory in bytes */
size_t phys_mem_pages; /* size of physical memory in pages */

char *kheap_start; /* The start address of the kernel heap */
char *kheap_end; /* The end address of the kernel heap */

/**
 * @brief Initialize basic memory contructs from the multiboot environment
 */
void mem_mb_init(struct multiboot_info *mb_info) {

  if (!(mb_info->flags & MULTIBOOT_INFO_MEMORY)) {
    panic("Multiboot info struct not reporting memory information!\n"
          "   mb_info->flags = 0x%08x\n", mb_info->flags);
  }

  phys_mem_bytes = MB(1) + (KB(1) * mb_info->mem_upper);
  phys_mem_pages = phys_mem_bytes / PAGE_SIZE;

  kprintf("RAM: %d MB\n", phys_mem_bytes / MB(1));

  /*
   * The kernel heap starts after the kernel image in memory
   */
  kheap_start = (char *) PAGE_ALIGN_UP(kimg_end);

  /*
   * If there are any modules installed (e.g. ramdisk) then we need to make
   * sure the kernel heap doesn't overwrite any modules.
   */
  if (mb_info->flags & MULTIBOOT_INFO_MODS) {
    unsigned i;

    for (i = 0; i < mb_info->mods_count; i++) {
      multiboot_module_t *m = ((multiboot_module_t *) mb_info->mods_addr) + i;
      if (m->mod_end > (size_t) kheap_start) {
        kheap_start = (char *) PAGE_ALIGN_UP(m->mod_end);
      }
    }
  }

  kheap_end = (char *) MB(16);

  kprintf("kernel image:  0x%08x, 0x%08x\n", (size_t) kimg_start, (size_t) kimg_end);
  kprintf("kernel heap:   0x%08x, 0x%08x\n", kheap_start, kheap_end);
}
