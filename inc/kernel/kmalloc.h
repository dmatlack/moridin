#include <stdlib.h>
#include <types.h>

#ifndef _MALLOC_WRAPPERS_H_
#define _MALLOC_WRAPPERS_H_

void kmalloc_heap_dump(void);

int   kmalloc_init(void);
//void *kmalloc(size_t size);
//void *kmemalign(size_t alignment, size_t size);
//void *kcalloc(size_t nelt, size_t eltsize);
//void *krealloc(void *buf, size_t new_size);
//void  kfree(void *buf);
void *kmalloc(size_t size);
void *kmemalign(size_t alignment, size_t size);
void  kfree(void *buf, size_t size);

#endif /* _MALLOC_WRAPPERS_H_ */
