#include <stdlib.h>
#include <types.h>

#ifndef _MALLOC_WRAPPERS_H_
#define _MALLOC_WRAPPERS_H_

void *kmalloc(size_t size);
void *kmemalign(size_t alignment, size_t size);
void *kcalloc(size_t nelt, size_t eltsize);
void *krealloc(void *buf, size_t new_size);
void  kfree(void *buf);
void *ksmalloc(size_t size);
void *ksmemalign(size_t alignment, size_t size);
void  ksfree(void *buf, size_t size);

#endif /* _MALLOC_WRAPPERS_H_ */
