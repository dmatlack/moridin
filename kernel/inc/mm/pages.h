/**
 * @file mm/pages.h
 *
 * @author David Matlack
 */
#ifndef __MM_PAGES_H__
#define __MM_PAGES_H__

#include <stddef.h>

int pages_init(void);
int alloc_pages(unsigned n, size_t *pages);
void free_pages(unsigned n, size_t *pages);

void reserve_kernel_pages(size_t paddr, size_t size);
unsigned num_kernel_pages(void);
size_t kernel_pages_pstart(void);


#endif /* !__MM_PAGES_H__ */
