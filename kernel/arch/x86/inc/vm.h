/**
 * @file x86/vm.h
 */
#ifndef __X86_VM_H__
#define __X86_VM_H__

#include <arch/x86/paging.h>

/**
 * @brief Convert the virtual address to the physical address it maps to.
 *
 * @return
 *    The physical address is the mapping exists.
 *    -1 (0xFFFFFFFF) otherwise.
 */
#define __phys(virt) to_phys((struct entry_table *) get_cr3(), (unsigned long) (virt))

/**
 * @brief Convert the virtual address to the page struct that it maps to.
 *
 * @return
 *    A pointer to the page struct if the mapping exists.
 *    NULL otherwise.
 */
#define __page(virt) ({ \
	unsigned long phys = __phys(virt); \
	phys == (unsigned long) -1 ? NULL : page_struct(phys); \
})


void *new_address_space(void);
static inline void *swap_address_space(void *new)
{
	void *old = (void *) (get_cr3() + CONFIG_KERNEL_VIRTUAL_START);
	set_cr3((u32) __phys(new));
	return old;
}

/**
 * @brief Map a physical page into the virtual address space.
 */
int mmu_map_page(void *page_dir, unsigned long virt, struct page *page, int flags);

/**
 * @brief Unmap a virtual page and return the physical page that it was
 * mapping to.
 */
struct page *mmu_unmap_page(void *page_dir, unsigned long virt);

/**
 * @brief Flush the contents of the TLB, invalidating all cached virtual
 * address lookups.
 */
static inline void tlb_flush(void) {
	set_cr3(get_cr3());
}

/**
 * @brief Invalidate a set of pages in the TLB. This should be called
 * after vm_map if you want to write to or read from the pages you just
 * mapped.
 */
static inline void tlb_invalidate(unsigned long addr, size_t size)
{
	unsigned long v;

	for (v = FLOOR(X86_PAGE_SIZE, addr); v < CEIL(X86_PAGE_SIZE, addr + size); v += X86_PAGE_SIZE) {
		__invlpg(v);
	}
}

#endif /* !__X86_VM_H__ */
