/**
 * @file mm/memory.h
 */
#ifndef __MM_MEMORY_H__
#define __MM_MEMORY_H__

#include <stddef.h>
#include <boot/multiboot.h>

#define PAGE_SIZE           KB(4)
#define PAGE_ALIGN_UP(n)    CEIL(PAGE_SIZE, n)
#define PAGE_ALIGN_DOWN(n)  FLOOR(PAGE_SIZE, n)
#define IS_PAGE_ALIGNED(n)  (PAGE_ALIGN_DOWN(n) == n)
#define PAGE_MASK           (~(PAGE_SIZE-1))

/*
 * The amount of memory mapped during early boot. This is the amount
 * of memory that can be addressed before vm_init().
 */
#define BOOT_PAGING_SIZE MB(16)

/*
 * Kernel Virtual Memory Layout:
 *
 *
 * CONFIG_KERNEL_VIRTUAL_START
 *        +---------------------------+ kdirect_start
 *        |     ... unused ...        |
 *   +1MB +---------------------------+ kimg_start
 *        |                           |
 *        |         kernel            |
 *        |          image            |
 *        |                           |
 *        +---------------------------+ kimg_end
 *        |                           |
 *        |         GRUB modules      |
 *        |         (e.g. initrd)     |
 *        |                           |
 *        +---------------------------+ kheap_start
 *        |                           |
 *        |         kernel            |   Calling kmalloc() returns an address in this
 *        |          heap             |   region.
 *        |                           |
 *        |                           |
 *        |                           |
 *        |                           |
 *        +---------------------------+ kheap_end, kdirect_end, kmap_start
 *        |                           |
 *        |         kernel            |   Calling kmap() creates a virtual memory mapping
 *        |         temporary         |   from an address in this range to a desired page
 *        |         mappings          |   somewhere in physical memory.
 *        |                           |
 *        +---------------------------+ kmap_end
 * CONFIG_KERNEL_VIRTUAL_END
 *
 *
 * kdirect: This is the region of virtual memory that is direct mapped to
 *          physical memory. Meaning kdirect_start maps directly to physical
 *          address 0 and kdirect_end maps directly to physical address
 *          kdirect_end - kdirect_start.
 *
 * kmap:    Unlike kdirect, the kmap region does map directly to physical
 *          memory. Instead, kmap contains temporary mappings to addresses
 *          all over physical memory.
 *
 */
extern char *kdirect_start, *kdirect_end;     /* kernel's direct mapped virtual memory */
extern char kimg_start[], kimg_end[];         /* kernel image (the entire loaded image) */
extern char ktext_start[], ktext_end[];       /* text (executable code) */
extern char krodata_start[], krodata_end[];   /* read-only initialized data */
extern char kdata_start[], kdata_end[];       /* initialized data */
extern char kbss_start[], kbss_end[];         /* unititialized data */
extern char *kheap_start, *kheap_end;         /* the kernel's heap (kmalloc) */
extern char *kmap_start, *kmap_end;           /* the kernel's temporary mappings (kmap) */

extern size_t phys_mem_bytes;
extern size_t phys_mem_pages;

void mem_mb_init(struct multiboot_info *mb_info);

#endif /* !__MM_MEMORY_H__ */
