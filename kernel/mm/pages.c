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

/**
 * @breif Initialize the physical page management system.
 */
int pages_init(void) {
  size_t p;

  __npages = phys_mem_bytes / PAGE_SIZE;
  __nfree = __npages;
  __index = 0;

  __pages = kmalloc(sizeof(struct page) * __npages);
  if (NULL == __pages) return ENOMEM;

  memset(__pages, 0, sizeof(struct page) * __npages);

  /*
   * Since the boot code maps the first 16 MB of memory, we'll set each
   * refcount for those pages to 1.
   */
  for (p = 0; p < MB(16) / PAGE_SIZE; p++) {
    __pages[p].count = 1;
  }
  __index = p;
  __nfree -= MB(16) / PAGE_SIZE;

  kprintf("system page list: %d pages (%d KB total)\n", __npages,
          __npages * sizeof(struct page) / KB(1));
  return 0;
}

size_t page_address(struct page *p) {
  return (((size_t) p - (size_t) __pages) / sizeof(struct page)) * PAGE_SIZE;
}

struct page *get_page(size_t address) {
  return __pages + (address / PAGE_SIZE);
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
  unsigned count, iter;

  TRACE("n=%d, pages=%p", n, pages);

  if (n > __nfree) {
    return ENOMEM;
  }
  if (0 == n) {
    return 0;
  }

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

  if (iter == __npages) {
    free_pages(count, pages);
    return ENOMEM;
  }

  return 0;
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