/**
 * @file pages.c
 *
 * @author David Matlack
 */
#include <mm/pages.h>
#include <mm/memory.h>

#include <kernel/kmalloc.h>
#include <kernel/kprintf.h> //remove me
#include <debug.h>

#include <errno.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

struct page {
  int count;
};

/*
 * A struct page for every physical page on the system.
 */
struct page *__pages;

static unsigned __npages;   /* size of __pages */
static unsigned __nfree;    /* number of available pages in __pages */
static unsigned __index;    /* last used lookup index in __pages */

static struct page *__kernel_pages;
static unsigned     __kernel_npages;

/**
 * @breif Initialize the physical page management system.
 */
void pages_init(void) {

  __npages = phys_mem_bytes / PAGE_SIZE;
  __nfree = __npages;
  __index = 0;

  __pages = kmalloc(sizeof(struct page) * __npages);
  if (NULL == __pages) {
    panic("Not enough memory to allocate the page list.");  
  }

  memset(__pages, 0, sizeof(struct page) * __npages);

  kprintf("system page list: %d pages (%d KB total)\n", __npages,
          __npages * sizeof(struct page) / KB(1));
}

size_t page_address(struct page *p) {
  return (((size_t) p - (size_t) __pages) / sizeof(struct page)) * PAGE_SIZE;
}

struct page *get_page(size_t address) {
  return __pages + (address / PAGE_SIZE);
}

/**
 * @brief Allocate a region of physical memory for use by the kernel.
 * 
 * This function is needed to remap the kernel after system startup.
 * After being loaded, one of the first things the kernel does is
 * enable paging, mapping the first 16 MB of memory. Later the kernel
 * needs to set up its virtual memory properly it will need to reserve
 * a set of physical pages.
 *
 * @param paddr The physical address of the first page to reserve.
 * @param size The size in bytes of the region to reserve.
 */
void alloc_kernel_pages(size_t paddr, size_t size) {
  unsigned i;

  TRACE("paddr=0x%x, size=0x%x", paddr, size);

  ASSERT(IS_PAGE_ALIGNED(paddr));
  ASSERT(IS_PAGE_ALIGNED(size));
  ASSERT_GREATER(__nfree, size / PAGE_SIZE);

  for (i = 0; i < size / PAGE_SIZE; i++) {
    struct page *p = get_page(paddr + (i*PAGE_SIZE));
    p->count = 1;
  }

  __kernel_pages = get_page(paddr);
  __kernel_npages = size / PAGE_SIZE;

  __nfree -= __kernel_npages;;
}

unsigned num_kernel_pages(void) {
  return __kernel_npages;
}

size_t kernel_pages_pstart(void) {
  return page_address(__kernel_pages);
}

static inline int __reserve_pages(unsigned n) {
  if (n > __nfree) {
    return ENOMEM;
  }
  __nfree -= n;
  return 0;
}

static inline void __fulfill_pages(unsigned n, size_t *pages) {
  unsigned count, iter;

  for (count = iter = 0; iter < __npages; iter++) {
    if (0 == __pages[__index].count) {
      struct page *p = __pages + __index;
      pages[count] = page_address(p);
      p->count++;
      count++;
      if (count == n) break;
    }

    __index = (__index + 1) % __npages;
  }

  /*
   * This function should never fail to fulfill all n pages.
   */
  ASSERT_EQUALS(count, n);
}

/**
 * @brief Request <n> pages be reserved for future use.
 *
 * This function decrements the counter that keeps track of how many pages
 * are available for use globally but does not actually increase the reference
 * counters of any pages.
 *
 * @return
 *    ENOMEM if there are no available pages
 *    0 on success
 */
int reserve_pages(unsigned n) {
  int ret;

  //lock
  ret = __reserve_pages(n);
  //unlock

  return ret;
}

/**
 * @brief Fulfill a previous request for reserved pages.
 *
 * This function assumes you previously called reserve_pages() and now want
 * some or all of those physical pages.
 */
void fulfill_pages(unsigned n, size_t *pages) {
  //lock
  __fulfill_pages(n, pages);
  //unlock
}

/**
 * @brief Request <n> arbitrarily placed pages throughout physical memory.
 *
 * @param n the number of pages to allocate
 * @param pages a buffer in which to store the physical address of each
 *  page allocated
 *
 * @return
 *    ENOMEM if there are no available pages
 *    0 on success
 */
int alloc_pages(unsigned n, size_t *pages) {
  int ret;

  TRACE("n=%d, pages=%p", n, pages);

  //lock

  ret = __reserve_pages(n);
  if (0 == ret) {
    __fulfill_pages(n, pages);
  }

  //unlock
  return ret;
}

/**
 * @brief Free <n> pages by decrementing their reference count.
 *
 * @param n The number of pages to free.
 * @param pages The physical address of the pages to free
 */
void free_pages(unsigned n, size_t *pages) {
  unsigned i;

  TRACE("n=%d, pages=%p", n, pages);

  for (i = 0; i < n; i++) {
    ASSERT_GREATEREQ(pages[i], MB(16)); // temporary check (don't free kernel pages)
    ASSERT_GREATER(get_page(pages[i])->count, 0);
    (get_page(pages[i]))->count--;
  }
}

/**
 * @brief Perform a "reference copy" of the given <n> physical pages.
 *
 * This function preforms 2 steps:
 *  1. Increment the reference count of each page provided.
 *  2. Reserve n pages for future fulfillment.
 *
 * This function is provided for copy-on-write.
 *
 * @return
 *    ENOMEM if n pages can't be reserved
 *    0 on success
 */
int refcopy_pages(unsigned n, size_t *pages) {
  int ret;

  //lock

  ret = __reserve_pages(n);
  if (0 == ret) {
    unsigned i;
    for (i = 0; i < n; i++) {
      struct page *p = get_page(pages[i]);

      ASSERT_GREATER(p->count, 0);
      p->count++;
    }
  }
  
  //unlock
 
  return ret;
}
