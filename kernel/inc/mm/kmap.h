/**
 * @file mm/kmap.h
 */
#ifndef __MM_KMAP_H__
#define __MM_KMAP_H__

#include <mm/pages.h>

void kmap_init(void);

void *kmap(struct page *);
void kunmap(void *);

#endif /* !__MM_KMAP_H__ */
