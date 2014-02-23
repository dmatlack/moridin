/**
 * @file mm/pages.h
 *
 * The general interface for page management is quite similar to Linux
 * (at least version 2.4.20).
 *
 * @author David Matlack
 */
#ifndef __MM_PAGES_H__
#define __MM_PAGES_H__

#include <stddef.h>

void pages_init(void);

struct page {
  int count;
};
size_t page_address(struct page *p);

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


void alloc_kernel_pages(size_t paddr, size_t size);
//TODO: struct page *alloc_pages_at(size_t addr, unsigned long n);


int alloc_pages(unsigned n, size_t *pages);
//TODO: struct page *alloc_pages(unsigned long n);


void free_pages(unsigned n, size_t *pages);
//TODO: void free_pages(struct page *pages, unsigned long n);

#endif /* !__MM_PAGES_H__ */
