/**
 * @file mm/memory.c
 */
#include <kernel/config.h>
#include <mm/memory.h>
#include <boot/multiboot.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>

size_t phys_mem_bytes; /* size of physical memory in bytes */
size_t phys_mem_pages; /* size of physical memory in pages */

char *kdirect_start;
char *kdirect_end;

/**
 * @brief Initialize basic memory contructs from the multiboot environment
 */
void mem_mb_init(struct multiboot_info *mb_info)
{
	ASSERT(mb_info->flags & MULTIBOOT_INFO_MEMORY);

	phys_mem_bytes = MB(1) + (KB(1) * mb_info->mem_upper);
	phys_mem_pages = phys_mem_bytes / PAGE_SIZE;

	INFO("RAM: %d MB", phys_mem_bytes / MB(1));

	kdirect_start = CONFIG_KERNEL_VIRTUAL_START;

	/*
	 * The kernel heap starts after the kernel image in memory
	 */
	kheap_start = (char *) PAGE_ALIGN_UP(kimg_end);

	/*
	 * If there are any modules installed (e.g. ramdisk) then we need to
	 * make sure the kernel heap doesn't overwrite any modules.
	 */
	if (mb_info->flags & MULTIBOOT_INFO_MODS) {
		unsigned i;

		for (i = 0; i < mb_info->mods_count; i++) {
			multiboot_module_t *m;

			m = ((multiboot_module_t *) mb_info->mods_addr) + i;
			if (m->mod_end > (size_t) kheap_start) {
				kheap_start =
					(char *) PAGE_ALIGN_UP(m->mod_end);
			}
		}
	}

	/*
	 * The kernel heap marks the end of the direct mapped pages of kernel
	 * memory. We statically allocate some fraction of physical memory to
	 * the kernel here. The rest goes to the user.
	 */
	kdirect_end = (char *) umin(CONFIG_KHEAP_MAX_END,
		CONFIG_KERNEL_VIRTUAL_START +
		PAGE_ALIGN_DOWN(phys_mem_bytes / 4));

	kheap_end = kdirect_end;

	/*
	 * The kmap region lives at the top of the kernel's virtual address
	 * space.
	 */
	kmap_start = kdirect_end;
	kmap_end = (char *) CONFIG_KERNEL_VIRTUAL_END;

	INFO("kimg:    0x%08x - 0x%08x", kimg_start, kimg_end);
	INFO("kheap:   0x%08x - 0x%08x", kheap_start, kheap_end);
	INFO("kmap:    0x%08x - 0x%08x", kmap_start, kmap_end);
}
