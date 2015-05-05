/**
 * @file kmalloc.c
 *
 * @brief kmalloc is the dynamic memory allocator in charge of the kernel
 * heap (kheap).
 *
 * TODO locking
 */
#include <mm/kmalloc.h>
#include <kernel/spinlock.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mm/lmm.h>
#include <mm/lmm_types.h>
#include <mm/memory.h>

/*
 * The lmm datastructures used to support the kernel heap.
 */
static lmm_t kheap_lmm;
static lmm_region_t global_region;
static struct spinlock kmalloc_lock = INITIALIZED_SPINLOCK;

char *kheap_start;         /* The start address of the kernel heap */
char *kheap_end;           /* The end address of the kernel heap */

char *kheap_early_start;   /* The start address of the kernel heap before vm_init() */
size_t kheap_early_size;   /* The size of the kernel's early heap */

static size_t kheap_used;  /* The number of bytes in use (allocated) */


void kmalloc_early_init(void)
{
	TRACE();

	lmm_init(&kheap_lmm);
	lmm_add_region(&kheap_lmm, &global_region, (size_t) 0, (size_t) -1,
		       0, 0);

	/*
	 * Set up an initial heap starting at kheap_start.
	 */
	kheap_early_start = kheap_start;

	/*
	 * The early heap size is limited by the amount of memory statically
	 * mapped during boot.
	 */
	kheap_early_size = BOOT_PAGING_SIZE - (size_t) kheap_early_start;

	lmm_add_free(&kheap_lmm, kheap_early_start, kheap_early_size);
	ASSERT_LESSEQ(kmalloc_bytes_free(), kheap_early_size);

	kheap_used = 0;
}

void kmalloc_late_init(void)
{
	char *start;
	size_t size;

	TRACE();

	/*
	 * We want to grow the kernel heap to kheap_end. Thus we add a
	 * new lmm_region that starts at the end of the early heap, and
	 * extends to kheap_end.
	 */
	start = kheap_early_start + kheap_early_size;
	size = (size_t) kheap_end - (size_t) start;

	lmm_add_free(&kheap_lmm, start, size);
	ASSERT_LESSEQ(kmalloc_bytes_free(),
		      (size_t) kheap_end - (size_t) kheap_start);
}

size_t kmalloc_bytes_free(void)
{
	return lmm_avail(&kheap_lmm, 0);
}

size_t kmalloc_bytes_used(void)
{
	return kheap_used;
}

static void *__kmalloc(size_t size)
{
	unsigned long flags;
	void *chunk;

	spin_lock_irq(&kmalloc_lock, &flags);

	chunk = lmm_alloc(&kheap_lmm, size, 0);
	if (!chunk)
		goto out;

	kheap_used += size;
out:
	spin_unlock_irq(&kmalloc_lock, flags);
	return chunk;
}

/**
 * @brief Allocate a chuck of memory of size <size>
 *
 * @return
 *    NULL if the memory could not be allocated
 *    otherwise return the address of the chunk
 */
void *kmalloc(size_t size)
{
	return __kmalloc(size);
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
void *kmemalign(size_t alignment, size_t size)
{
	unsigned long flags;
	unsigned shift;
	void *chunk;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
	/* Find the alignment shift in bits.  XXX use proc_ops.h  */
	for (shift = 0; (1 << shift) < alignment; shift++);

	spin_lock_irq(&kmalloc_lock, &flags);

	/*
	 * Allocate a chunk of LMM memory with the specified alignment shift
	 * and an offset such that the memory block we return will be aligned.
	 */
	chunk = lmm_alloc_aligned(&kheap_lmm, size, 0, shift, 0);
	if (!chunk)
		goto out;

#pragma GCC diagnostic pop

	kheap_used += size;
out:
	spin_unlock_irq(&kmalloc_lock, flags);
	return chunk;
}

/**
 * @brief Free a region of memory
 *
 * @param buf The address of the memory to free
 * @param size The size of the memory to free
 */
void kfree(void *buf, size_t size)
{
	unsigned long flags;

	kheap_used -= size;

	spin_lock_irq(&kmalloc_lock, &flags);

	lmm_free(&kheap_lmm, buf, size);

	spin_unlock_irq(&kmalloc_lock, flags);
}
