/**
 * @file mm/kmalloc.h
 */
#ifndef __MM_KMALLOC_H__
#define __MM_KMALLOC_H__

#include <stdlib.h>
#include <types.h>

void kmalloc_early_init(void);
void kmalloc_late_init(void);

void kmalloc_dump(void);

size_t kmalloc_bytes_free(void);
size_t kmalloc_bytes_used(void);

void *kmalloc(size_t size);
void *kmemalign(size_t alignment, size_t size);
void  kfree(void *buf, size_t size);

#endif /* !__MM_KMALLOC_H__ */
