/**
 * @file physmem.c
 *
 * @author David Matlack
 */
#include <mm/physmem.h>

#include <kernel/kmalloc.h>
#include <kernel/kprintf.h> //remove me
#include <debug.h>

#include <stddef.h>
#include <assert.h>

struct pmem_map __pmem;

/**
 * @brief Initialize the the pmem_map struct that contains all of the
 * basic physical memory attributes needed by the kernel.
 *
 * This function does NOT initialize the physical memory allocator, which
 * requires dynamic memory allocation and therefore virtual memory.
 * 
 * @param mem_max The maximum available physical memory on the system.
 * @param page_size The size of memory pages on the system.
 * @param kimg_start The start address of the kernel image in memory.
 * @param kimg_end The end address of the kernel image in memory.
 */
int pmem_bootstrap(size_t max_mem, size_t page_size,
                   char kimg_start[], char kimg_end[]) {
  size_t kmem_size;
  size_t umem_size;
  size_t mem_avail;

  __pmem.max_mem = max_mem;
  __pmem.page_size = page_size;

  __pmem.kernel_image_start = PAGE_ALIGN_DOWN((size_t) kimg_start);
  __pmem.kernel_image_end = PAGE_ALIGN_UP((size_t) kimg_end);
  
  PMEM_ZONE_BIOS->address = 0;
  PMEM_ZONE_BIOS->size = MB(1);
  PMEM_ZONE_BIOS->dbgstr = "PMEM_ZONE_BIOS";

  PMEM_ZONE_DMA->address = MB(1);
  PMEM_ZONE_DMA->size = MB(15);
  PMEM_ZONE_DMA->dbgstr = "PMEM_ZONE_DMA";

  /*
   * We need to divide up the remaining memory between the user and the 
   * kernel without giving too much memory to the user such that the kernel
   * can't do very much and not giving too much to the kernel such that only
   * few processes can run.
   */
  mem_avail = max_mem - MB(16);
  assert(mem_avail >= CONFIG_MIN_USER_MEM + CONFIG_MIN_KERNEL_MEM);

  umem_size = PAGE_ALIGN_DOWN(mem_avail / 4 * 3);
  kmem_size = PAGE_ALIGN_DOWN(mem_avail - umem_size);

  if (kmem_size > CONFIG_MAX_KERNEL_MEM) {
    kmem_size = CONFIG_MAX_KERNEL_MEM;
    umem_size = mem_avail - kmem_size;
  }

  assert(kmem_size > CONFIG_MIN_KERNEL_MEM);
  assert(umem_size > CONFIG_MIN_USER_MEM);

  PMEM_ZONE_USER->address = MB(16);
  PMEM_ZONE_USER->size = umem_size;
  PMEM_ZONE_USER->dbgstr = "PMEM_ZONE_USER";

  PMEM_ZONE_KERNEL->address = PMEM_ZONE_USER->address + PMEM_ZONE_USER->size;
  PMEM_ZONE_KERNEL->size = kmem_size;
  PMEM_ZONE_KERNEL->dbgstr = "PMEM_ZONE_KERNEL";

  return 0;
}


/**
 * @breif Initialize the physical memory management system.
 *
 * @warning Must be called AFTER pmem_bootstrap().
 * @warning DO NOT CALL THIS FUNCTION BEFORE VIRTUAL MEMORY AND KMALLOC
 * HAVE BEEN SET UP.
 */
int pmem_init(void) {
  int z;
  int p;

  TRACE("void");
  LOG_PMEM_ZONE(PMEM_ZONE_BIOS);
  LOG_PMEM_ZONE(PMEM_ZONE_DMA);
  LOG_PMEM_ZONE(PMEM_ZONE_KERNEL);
  LOG_PMEM_ZONE(PMEM_ZONE_USER);

  /*
   * kmalloc each zone's page list and set the reference counts of each
   * page to zero
   */
  for (z = 0; z < PMEM_NUM_ZONES; z++) {
    struct pmem_zone *zone = PMEM_ZONE(z);

    zone->num_pages = zone->size / PAGE_SIZE;
    zone->num_free = zone->num_pages;
    zone->page_index = 0;
    zone->pages = kmalloc(zone->num_pages * sizeof(struct pmem_page));

    /*
     * zone->address and zone->size should have already been initialize in
     * pmem_bootstrap()
     */

    for (p = 0; p < zone->num_pages; p++) {
      zone->pages[p].refcount = 0;
    }
  }

  return 0;
}

static inline void *__page_paddr(size_t address, int page_index) {
  return (void *) (address + (page_index * PAGE_SIZE));
}

static inline int __page_index(size_t page_addr, size_t zone_addr) {
  return (int) ((page_addr - zone_addr) / PAGE_SIZE); 
}

/**
 * @brief Allocate (reserve) an entire zone of memory. This is used by 
 * the kernel vm bootstrap code to reserve the entire kernel zone.
 */
void pmem_alloc_zone(struct pmem_zone *zone) {
  int i;

  for (i = 0; i < zone->num_pages; i++) {
    assert(zone->pages[i].refcount == 0);
    zone->pages[i].refcount = 1;
  }
}

/**
 * @brief Allocate <num_to_alloc> physical pages of memory in the given zone, 
 * passing back the address of each alloc'ed page back to the caller in the 
 * <pages> array.
 *
 * @warning <pages> must have enough room for num_pages pointers
 *
 * @return 0 on success, < 0 on error
 */
int pmem_alloc(void **pages, int num_to_alloc, struct pmem_zone *zone) {
  struct pmem_page *pg;
  int num_alloced = 0;

  if (zone->num_free < num_to_alloc) {
    return -1;
  }

  for (; zone->page_index < zone->num_pages;
       zone->page_index = (zone->page_index + 1) % zone->num_pages) {
    pg = &zone->pages[zone->page_index];
    
    if (pg->refcount == 0) {
      pg->refcount = 1;
      pages[num_alloced] = __page_paddr(zone->address, zone->page_index);
      num_alloced++;
    }

    if (num_alloced == num_to_alloc) {
      break;
    }
  }

  return 0;
}

/**
 * @brief Decrement the reference count of the given <num_to_free>
 * pages.
 */
void pmem_free(void **pages, int num_to_free, struct pmem_zone *zone) {
  struct pmem_page *pg;
  int page_index;
  int i;

  for (i = 0; i < num_to_free; i++) {
    page_index = __page_index((size_t) pages[i], (size_t) zone->address);
    pg = &zone->pages[page_index];

    if (pg->refcount > 0) {
      pg->refcount--;
    }
    else {
      panic("Trying to free page %d (paddr 0x%08x) in zone %s with "
            "refcount %d.\n", page_index, pages[i], zone->dbgstr, 
            pg->refcount);
    }
  }
  
}

/**
 * @brief Print out the global pmem_map struct using the given print 
 * function.
 */
void pmem_map_dump(printf_f p) {
  int i;

#define PRINT_PADDR_MB(a) p("0x%08x (%d MB)\n", (a), (a) / MB(1))
#define PRINT_PADDR_KB(a) p("0x%08x (%d KB)\n", (a), (a) / KB(1))
  p("=== Physical Memory Layout ===\n");
  p("max_mem:   "); PRINT_PADDR_MB(__pmem.max_mem);
  p("page_size: "); PRINT_PADDR_KB(__pmem.page_size);
  p("\n");
  p("kernel_image_start: 0x%08x\n", __pmem.kernel_image_start);
  p("kernel_image_end:   0x%08x\n", __pmem.kernel_image_end);
  p("\n");
  for (i = 0; i < PMEM_NUM_ZONES; i++) {
    struct pmem_zone *z = __pmem.zones + i;
    p("[%d] %s\n", i, z->dbgstr);
    p("   size  = "); PRINT_PADDR_MB(z->size);
    p("   end   = "); PRINT_PADDR_MB(z->address + z->size);
    p("   start = "); PRINT_PADDR_MB(z->address);
    p("\n");
  }
#undef PRINT_PADDR_MB
#undef PRINT_PADDR_KB

}

