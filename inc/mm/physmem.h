/**
 * @file mm/physmem.h
 *
 * @brief All things related to physical memory: the layout of physical
 * memory, the size of physical memory, the system page size, the various
 * memory zones, the management of physical pages, etc.
 *  
 * @author David Matlack
 */
#ifndef __MM_PHYSMEM_H__
#define __MM_PHYSMEM_H__

#include <stddef.h>
#include <types.h>
#include <kernel/config.h>

struct pmem_page {
  int refcount;
};

struct pmem_zone {
  size_t address;
  size_t size;
  /*
   * We store an integer for every page in this zone. This allows us to keep 
   * track of the number of references to the page, where a "reference" means
   * that something is using the page. Thus is reference == 0, the page is
   * available (free). If reference > 0, the page is in use by one or more
   * things (probably processes).
   */
  struct pmem_page *pages;
  int num_pages;
  int num_free;
  int page_index;

  /*
   * debugging variables
   */
  const char *dbgstr;
};

/*
 * Pages in the kernel's address space will map to pages in this zone. If the
 * kernel in permanently mapped in memory (most likely), then this zone will
 * be directly mapped to a contiguous region of virtual memory of the same 
 * size.
 */ 
#define ZONE_KERNEL_IDX 0

/*
 * All pages needed to back user procces's address space will come from this
 * zone.
 */
#define ZONE_USER_IDX   1

/*
 * This zone is reserved for Direct Memory Access devices. It typically covers
 * memory up to 16 MB. The kernel image (text and data) may also be in this
 * zone.
 */
#define ZONE_DMA_IDX    2

/*
 * The BIOS reserves the first 1 MB of memory. Technically we could reclaim it
 * but we will give it it's own zone and leave it alone.
 */
#define ZONE_BIOS_IDX   3

#define PMEM_NUM_ZONES 4


struct pmem_map {
  /*
   * The maximum available PHYSICAL MEMORY. This is determined by how much
   * RAM you have.
   */
  size_t max_mem;
  size_t page_size;
  size_t kernel_image_start;
  size_t kernel_image_end;
  struct pmem_zone zones[PMEM_NUM_ZONES];
};

extern struct pmem_map __pmem;

/*
 * convenient access macros
 */
#define PAGE_SIZE        (__pmem.page_size)
#define PMEM_ZONE_KERNEL (__pmem.zones + ZONE_KERNEL_IDX)
#define PMEM_ZONE_DMA    (__pmem.zones + ZONE_DMA_IDX)
#define PMEM_ZONE_USER   (__pmem.zones + ZONE_USER_IDX)
#define PMEM_ZONE_BIOS   (__pmem.zones + ZONE_BIOS_IDX)
#define PMEM_ZONE(i)     (__pmem.zones + i)

#define PAGE_ALIGN_UP(n) CEIL(PAGE_SIZE, n)
#define PAGE_ALIGN_DOWN(n) FLOOR(PAGE_SIZE, n)

#define LOG_PMEM_ZONE( zone_macro )\
  INFO(#zone_macro": address=0x%08x, size=0x%08x (%d MB)",\
       zone_macro->address, zone_macro->size, zone_macro->size / MB(1));

int pmem_bootstrap(size_t max_mem, size_t page_size,
                   char kimg_start[], char kimg_end[]);
int pmem_init(void);
void pmem_alloc_zone(struct pmem_zone *zone);
int pmem_alloc(void **pages, int num_to_alloc, struct pmem_zone *zone);
void pmem_free(void **pages, int num_to_free, struct pmem_zone *zone);

void pmem_map_dump(printf_f p);

#endif /* !__MM_MEM_H__ */
