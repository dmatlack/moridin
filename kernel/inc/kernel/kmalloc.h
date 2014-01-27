#include <stdlib.h>
#include <types.h>

#ifndef _MALLOC_WRAPPERS_H_
#define _MALLOC_WRAPPERS_H_

void kmalloc_early_init(size_t start, size_t size);
void kmalloc_late_init(size_t new_end);
void kmalloc_dump(void);
size_t kmalloc_bytes_free(void);
size_t kmalloc_bytes_used(void);

void *kmalloc(size_t size);
void *kmemalign(size_t alignment, size_t size);
void  kfree(void *buf, size_t size);

#endif /* _MALLOC_WRAPPERS_H_ */
