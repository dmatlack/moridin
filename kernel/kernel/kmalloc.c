/**
 * @file kmalloc.c
 *
 * TODO locking
 *
 * @author David Matlack (dmatlack)
 * @bug No known bugs
 */
#include <kernel/kmalloc.h>
#include <kernel/debug.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mm/lmm.h>
#include <mm/lmm_types.h>
#include <mm/memory.h>

static lmm_t        kernel_lmm; // = LMM_INITIALIZER;
static lmm_region_t global_region;
static size_t       __kmalloc_total;

/**
 * @brief Initialize kmalloc and set up the kernel heap to extend
 * from <start> to <start>+<size>.
 *
 * This function is used to allocate a small heap for system startup.
 */
void kmalloc_early_init(size_t start, size_t size) {
  TRACE("start=0x%0x, size=0x%x", start, size);

  __kmalloc_total = 0;

  lmm_init(&kernel_lmm);
  lmm_add_region(&kernel_lmm, &global_region, (size_t) 0, (size_t) -1, 0, 0);
  lmm_add_free(&kernel_lmm, (void *) start, size);
}

/**
 * @brief Grow the kernel heap to address <new_end>.
 *
 * This function is called after the kernel has bootstrapped it's virtual
 * memory management.
 */
void kmalloc_late_init(size_t new_end) {
  size_t start;

  TRACE("new_end=0x%x", new_end);

  start = (size_t) kheap_end;

  lmm_add_free(&kernel_lmm, (void *) start, new_end - start);

  kheap_end = (char *) new_end;
}

void kmalloc_dump(void) {
  lmm_dump(&kernel_lmm);
}

size_t kmalloc_bytes_free(void) {
  return lmm_avail(&kernel_lmm, 0);
}

size_t kmalloc_bytes_used(void) {
  return __kmalloc_total;
}

static void *__kmalloc(size_t size) {
	void *chunk;

	if (!(chunk = lmm_alloc(&kernel_lmm, size, 0)))
        return NULL;

  __kmalloc_total += size;
	return chunk;
}

/**
 * @brief Allocate a chuck of memory of size <size>
 *
 * @return
 *    NULL if the memory could not be allocated
 *    otherwise return the address of the chunk
 */
void *kmalloc(size_t size) {
  void* addr;

  addr = __kmalloc(size);

  return addr;
}

/**
 * @brief Allocate a chunk of memory of size <size> aligned to
 * <alignment>.
 *
 * @param alignment The request alignment. kmalloc will align your allocation
 *  to the lowest power of 2 greater than <alignment>.
 *
 * @param size The number of bytes to allocate
 *
 * @return
 *    NULL if the allocation failed
 *    otherwise return the address of the chunk
 */
void *kmemalign(size_t alignment, size_t size) {
	unsigned shift;
	void *chunk;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
	/* Find the alignment shift in bits.  XXX use proc_ops.h  */
	for (shift = 0; (1 << shift) < alignment; shift++);

	/*
	 * Allocate a chunk of LMM memory with the specified alignment shift
	 * and an offset such that the memory block we return will be aligned.
	 */
	if (!(chunk = lmm_alloc_aligned(&kernel_lmm, size, 0, shift, 0)))
		return NULL;
#pragma GCC diagnostic pop

  __kmalloc_total += size;
	return chunk;
}

/**
 * @brief Free a region of memory
 *
 * @param buf The address of the memory to free
 * @param size The size of the memory to free
 */
void kfree(void *buf, size_t size) {
  __kmalloc_total -= size;
	lmm_free(&kernel_lmm, buf, size);
}
