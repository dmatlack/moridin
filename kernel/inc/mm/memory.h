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

extern char kimg_start[];
extern char kimg_end[];
extern char ktext_start[];
extern char ktext_end[];
extern char krodata_start[];
extern char krodata_end[];
extern char kdata_start[];
extern char kdata_end[];
extern char kbss_start[];
extern char kbss_end[];

extern char *kheap_start;
extern char *kheap_end;

extern char boot_stack_top[];
extern char boot_stack_bottom[];
extern char boot_page_dir[];

extern size_t phys_mem_bytes;
extern size_t phys_mem_pages;

void mem_mb_init(struct multiboot_info *mb_info);

#endif /* !__MM_MEMORY_H__ */
