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
struct page *pages;

static int  npages;       /* size of pages */
static int  nfree_pages;  /* number of available pages in pages */
static int  pages_index;  /* last used lookup index in pages */

/**
 * @breif Initialize the physical page management system.
 */
int pages_init(void) {
  size_t p;

  npages = phys_mem_bytes / PAGE_SIZE;
  nfree_pages = npages;
  pages_index = 0;

  pages = kmalloc(sizeof(struct page) * npages);
  if (NULL == pages) return ENOMEM;

  memset(pages, 0, sizeof(struct page) * npages);

  /*
   * Since the boot code maps the first 16 MB of memory, we'll set each
   * refcount for those pages to 1.
   */
  for (p = 0; p < MB(16) / PAGE_SIZE; p++) {
    pages[p].count = 1;
  }
  nfree_pages -= MB(16) / PAGE_SIZE;

  kprintf("system page table: %d entries (%d KB)\n", npages,
          npages * sizeof(struct page) / KB(1));
  return 0;
}
