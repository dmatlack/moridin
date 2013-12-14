/**
 * @file kmalloc.c
 *
 * TODO locking
 *
 * @author David Matlack (dmatlack)
 * @bug No known bugs
 */
#include <kernel/kmalloc.h>
#include <debug.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mm/physmem.h> //FIXME remove me once vm bootstrap works
#include <mm/vm.h>
#include <mm/lmm.h>
#include <mm/lmm_types.h>

lmm_t kernel_lmm = LMM_INITIALIZER;
lmm_region_t global_region;

int kmalloc_init(void) {
  size_t kmalloc_start, kmalloc_size;

  TRACE("void");

  lmm_init(&kernel_lmm);

  /*
   * add an lmm_region that covers the entire possible address space (0, -1)
   */
  lmm_add_region(&kernel_lmm, &global_region, (size_t) 0, (size_t) -1, 0, 0);

  /*
   * We can do dynamic memory allocation in region of memory starting right 
   * after the kernel image.
   *
   * FIXME: this only works if the address of the kernel image in physical
   * memory is the same as its address in virtual memory!!!!
   */
  kmalloc_start = VM_ZONE_KERNEL->address + 
    (KERNEL_IMAGE_END - PMEM_ZONE_KERNEL->address);

  kmalloc_size = PMEM_ZONE_KERNEL->size -
    (KERNEL_IMAGE_END - PMEM_ZONE_KERNEL->address);

  DEBUG("    kmalloc_start=0x%08x", kmalloc_start);
  DEBUG("    kmalloc_size=0x%08x", kmalloc_size);
  DEBUG("    kmalloc_end=0x%08x", kmalloc_start + kmalloc_size);

  lmm_add_free(&kernel_lmm, (void*) kmalloc_start, kmalloc_size);

  return 0;
}

void kmalloc_heap_dump(void) {
  lmm_dump(&kernel_lmm);
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
  void* addr;

  TRACE("size=0x%08x", size);

  addr = __kmalloc(size);

  DEBUG("    return %p", addr);
  return addr;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
void *kmemalign(size_t alignment, size_t size) {
	unsigned shift;
	void *chunk;

  TRACE("alignment=0x%08x, size=0x%08x", alignment, size);

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
  TRACE("buf=%p, size=0x%08x", buf, size);
	lmm_free(&kernel_lmm, buf, size);
}
