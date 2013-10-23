/**
 * @file kmalloc.c
 *
 * TODO locking
 *
 * @author David Matlack (dmatlack)
 * @bug No known bugs
 */
#include <kernel/kmalloc.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mm/lmm.h>
#include <mm/lmm_types.h>
lmm_t kernel_lmm = LMM_INITIALIZER;
lmm_region_t global_region;

#include <mm/mem.h>

int kmalloc_init(void) {
  lmm_init(&kernel_lmm);

  /*
   * add an lmm_region that covers the entire possible address space (0, -1)
   */
  lmm_add_region(&kernel_lmm, &global_region, (size_t) 0, (size_t) -1, 0, 0);

  lmm_add_free(&kernel_lmm, (void *) KMEM_START, 
    (size_t) KMEM_END - KMEM_START);

  return 0;
}

//void *__kmalloc(size_t size) {
//	size_t *chunk;
//
//	size += sizeof(size_t);
//
//	if (!(chunk = lmm_alloc(&kernel_lmm, size, 0)))
//		return 0;
//
//	*chunk = size;
//	return chunk+1;
//}
//
///* safe versions of malloc functions */
//void *kmalloc(size_t size) {
//  return __kmalloc(size);
//}
//
////FIXME
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wsign-compare"
//void *kmemalign(size_t alignment, size_t size) {
//	unsigned shift;
//	size_t *chunk;
//
//	/* Find the alignment shift in bits.  XXX use proc_ops.h  */
//	for (shift = 0; (1 << shift) < alignment; shift++);
//
//	/*
//	 * Allocate a chunk of LMM memory with the specified alignment shift
//	 * and an offset such that the memory block we return will be aligned
//	 * after we add our size field to the beginning of it.
//	 */
//	size += sizeof(size_t);
//
//	if (!(chunk = lmm_alloc_aligned(&kernel_lmm, size, 0, shift,
//					   (1 << shift) - sizeof(size_t))))
//        return NULL;
//
//	*chunk = size;
//	return chunk+1;
//}
//#pragma GCC diagnostic pop
//
//void *kcalloc(size_t nelt, size_t eltsize) {
//	size_t allocsize = nelt * eltsize;
//
//	void *ptr = __kmalloc(allocsize);
//	if (!ptr)
//		return NULL;
//
//	memset(ptr, 0, allocsize);
//
//	return ptr;
//}
//
//void *krealloc(void *buf, size_t new_size) {
//	vm_size_t *op;
//	vm_size_t old_size;
//	vm_size_t *np;
//
//	if (buf == 0)
//		return __kmalloc(new_size);
//
//	op = (vm_size_t*)buf;
//	old_size = *--op;
//
//	new_size += sizeof(vm_size_t);
//	if (!(np = lmm_alloc(&kernel_lmm, new_size, 0)))
//	    return NULL;
//
//	memcpy(np, op, old_size < new_size ? old_size : new_size);
//
//	lmm_free(&kernel_lmm, op, old_size);
//	
//	*np++ = new_size;
//	return np;
//}
//
//void kfree(void *chunk_ptr) {
//	size_t *chunk = (size_t*)chunk_ptr - 1;
//	lmm_free(&kernel_lmm, chunk, *chunk);
//}

void *__kmalloc(size_t size) {
	void *chunk;

	if (!(chunk = lmm_alloc(&kernel_lmm, size, 0)))
        return NULL;

	return chunk;
}

void *kmalloc(size_t size) {
  return __kmalloc(size);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
void *kmemalign(size_t alignment, size_t size) {
	unsigned shift;
	void *chunk;

	/* Find the alignment shift in bits.  XXX use proc_ops.h  */
	for (shift = 0; (1 << shift) < alignment; shift++);

	/*
	 * Allocate a chunk of LMM memory with the specified alignment shift
	 * and an offset such that the memory block we return will be aligned.
	 */
	if (!(chunk = lmm_alloc_aligned(&kernel_lmm, size, 0, shift, 0)))
		return NULL;

	return chunk;
}
#pragma GCC diagnostic pop

void *kcalloc(size_t size) {
  void *ret = __kmalloc(size);
  if (ret != NULL)
    memset(ret, 0, size);
  return ret;
}

void kfree(void *buf, size_t size) {
	lmm_free(&kernel_lmm, buf, size);
}
