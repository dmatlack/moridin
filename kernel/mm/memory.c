/**
 * @file mm/memory.c
 */
#include <mm/memory.h>
#include <boot/multiboot.h>
#include <stddef.h>
#include <assert.h>
#include <kernel.h>

/*
 * The size of physical memory in bytes
 */
size_t phys_mem_bytes;

/**
 * @brief Initialize basic memory contructs from the multiboot environment
 */
void mem_mb_init(struct multiboot_info *mb_info) {

  if (!(mb_info->flags & MULTIBOOT_INFO_MEMORY)) {
    panic("Multiboot info struct not reporting memory information!\n"
          "   mb_info->flags = 0x%08x\n", mb_info->flags);
  }

  phys_mem_bytes = MB(1) + (KB(1) * mb_info->mem_upper);

  kprintf("RAM: %d MB\n", phys_mem_bytes / MB(1));

  //TODO maybe parse the mmap
}
