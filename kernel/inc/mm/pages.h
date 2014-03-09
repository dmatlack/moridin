/**
 * @file mm/pages.h
 *
 * The general interface for page management is quite similar to Linux
 * (at least version 2.4.20).
 *
 */
#ifndef __MM_PAGES_H__
#define __MM_PAGES_H__

#include <stddef.h>
#include <mm/memory.h>

void pages_init(void);

struct page {
  int count;
};

extern struct page *phys_pages;

#define page_address(_page) \
  ((((size_t) (_page) - (size_t) phys_pages) / sizeof(struct page)) * PAGE_SIZE)

#define page_struct(_address) \
  (phys_pages + ((_address) / PAGE_SIZE))

#include <arch/atomic.h>
#define page_get(_page) (atomic_add(&((_page)->count),  1))
#define page_put(_page) (atomic_add(&((_page)->count), -1))

struct page_zone {
  struct page *pages; /* array of all pages in the zone */
  unsigned long num_pages; /* total number of pages in the zone */
  unsigned long num_free; /* number of free pages in the zone */
  unsigned long index; /* allows searches to pick up where the last left off */

  // lock struct thingy
};
#define MAX_ZONES 1

#define ZONE_START_PAGE_ADDR(zone) \
  (page_address((zone)->pages))

#define ZONE_END_PAGE_ADDR(zone) \
  (page_address((zone)->pages + ((zone)->num_pages - 1)))


struct page *alloc_pages_at(size_t addr, unsigned long n);

struct page *alloc_pages(unsigned long n);
#define alloc_page() alloc_pages(1)

void free_pages(struct page *pages, unsigned long n);
#define free_page(_p) free_pages(_p, 1)

#endif /* !__MM_PAGES_H__ */
