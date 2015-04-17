/**
 * @file x86/vm.c
 *
 * @brief x86 Virtual Memory Implementation
 *
 */
#include <arch/vm.h>
#include <arch/page.h>
#include <arch/reg.h>
#include <arch/cpu.h>

#include <mm/kmalloc.h>

#include <mm/memory.h>
#include <mm/vm.h>
#include <mm/pages.h>

#include <stddef.h>
#include <assert.h>
#include <kernel/debug.h>
#include <errno.h>

static inline bool is_page_aligned(unsigned long addr)
{
	return FLOOR(X86_PAGE_SIZE, addr) == addr;
}

static inline bool is_kernel_entry(entry_t *e)
{
	return entry_is_global(e);
}

/**
 * @brief Allocate a page directory to be used in a new address space.
 *
 * @return NULL if allocating a page of memory failed, the address of the 
 * page directory otherwise.
 */
void *new_address_space(void)
{
	struct entry_table *page_directory;

	page_directory = new_entry_table();
	if (!page_directory) {
		return NULL;
	}

	return (void *) page_directory;
}

static inline void free_page_table_pde(entry_t *pde)
{
	ASSERT(entry_is_present(pde));
	ASSERT(!is_kernel_entry(pde));

	free_entry_table(entry_pt(pde));
}

void free_address_space(void *mmu)
{
	struct entry_table *page_directory = mmu;
	entry_t *pde;

	/* should not be destroying the page tables while they are in use */
	ASSERT_NOTEQUALS(page_directory, get_cr3());

	foreach_entry(pde, page_directory) {
		/* don't free kernel page tables */
		if (is_kernel_entry(pde))
			continue;

		if (entry_is_present(pde))
			free_page_table_pde(pde);
	}

	free_entry_table(page_directory);
}

/**
 * @brief This function looks at the flags (see mm/vm.h) and sets the
 * correct bits in the page (table|directory) entry.
 */
void entry_set_flags(entry_t *entry, int flags)
{
	/* zero out all old flags */
	*entry &= ENTRY_ADDR_MASK;

	if (flags & VM_W)
		entry_set_readwrite(entry);
	else
		entry_set_readonly(entry);

	if (flags & VM_S)
		entry_set_supervisor(entry);
	else
		entry_set_user(entry);

	if (flags & VM_G)
		entry_set_global(entry);

	if (flags & VM_P)
		entry_set_present(entry);
	else
		entry_set_absent(entry);
}

/**
 * @brief Convert a virtual address to the physical address it is mapped
 * to.
 */
unsigned long to_phys(struct entry_table *pd, unsigned long virt)
{
	if (kernel_address(virt)) {
		return virt - CONFIG_KERNEL_VIRTUAL_START;
	}
	else {
#define VIRT_NOT_MAPPED ((unsigned long) -1)
		struct entry_table *pt;
		entry_t *pde, *pte;

		pde = get_pde(pd, virt);

		if (!entry_is_present(pde)) {
			return VIRT_NOT_MAPPED;
		}

		pt = entry_pt(pde);
		pte = get_pte(pt, virt);

		if (!entry_is_present(pte)) {
			return VIRT_NOT_MAPPED;
		}

		return entry_phys(pte) + PHYS_OFFSET(virt);
	}
}

/**
 * @brief Free the empty page tables referenced by the page directory. Only
 * look at PDEs marked with ENTRY_TABLE_UNMAP to check if empty.
 */
void free_marked_page_tables(struct entry_table *pd)
{
	entry_t *pde, *pte;

	foreach_entry(pde, pd) {

		/*
		 * if we marked the page _directory_ entry, then we need to
		 * check if the corresponding page table is empty, and if so
		 * free it.
		 */
		if (entry_is_present(pde) && (*pde & ENTRY_TABLE_UNMAP)) {
			struct entry_table *pt = entry_pt(pde);
			bool page_table_empty = true;

			foreach_entry(pte, pt) {
				if (entry_is_present(pte)) {
					page_table_empty = false;
					break;
				}
			}

			if (page_table_empty) {
				free_page_table_pde(pde);
				entry_set_absent(pde);
			}

			*pde &= ~ENTRY_TABLE_UNMAP;
		}
	}
}

struct page *unmap_page_pde(entry_t *pde, unsigned long virt)
{
	entry_t *pte;

	/*
	 * Mark the page _directory_ entry so we know that we unmapped a page
	 * in this page table and we can free it later (if it's empty). Don't
	 * mark the page directory entry if it global though because it is
	 * being shared with other processes.
	 */
	*pde |= ENTRY_TABLE_UNMAP;

	pte = get_pte(entry_pt(pde), virt);

	/*
	 * This page wasn't mapped in the first place. Return NULL to indicate
	 * no page was unmapped.
	 */
	if (!entry_is_present(pte)) {
		ERROR("Trying to unmap page that was never mapped. virt 0x%lx, pte 0x%lx",
		      virt, pte);
		return NULL;
	}

	/*
	 * Mark the page _table_ entry as not present, effectively unmapping
	 * the page.
	 */
	entry_set_absent(pte);

	return page_struct(entry_phys(pte));
}

struct page *mmu_unmap_page(void *pd, unsigned long virt)
{
	struct page *page = NULL;
	entry_t *pde;
	int i;

	TRACE("pd=%p, virt=0x%x", pd, virt);

	for (i = 0; i < PAGE_SIZE / X86_PAGE_SIZE; i++) {
		virt += i * X86_PAGE_SIZE;

		pde = get_pde(pd, virt);

		if (!entry_is_present(pde))
			continue;

		if (!page)
			page = unmap_page_pde(pde, virt);
	}

	free_marked_page_tables(pd);

	return page;
}

/**
 * @brief Maps virt to phys with the given flags.
 *
 * @return
 *    0 on success, non-0 if the mapping could not be completed.
 *
 *    ENOMEM if mapping this page requires allocating a new page
 *      table data structure and the system has run out of available
 *      memory.
 */
static int map(struct entry_table *pd, unsigned long virt,
	       unsigned long phys, int flags)
{
	struct entry_table *pt;
	entry_t *pde, *pte;

	pde = get_pde(pd, virt);

	/*
	 * Create a page table if there is not one already present for the
	 * 4MB chunk containing this page.
	 */
	if (!entry_is_present(pde)) {
		pt = new_entry_table();
		if (!pt) {
			return ENOMEM;
		}

		entry_set_addr(pde, __phys(pt));
		entry_set_flags(pde, flags | VM_P);
	}

	pt = (struct entry_table *) entry_get_addr(pde);
	ASSERT(entry_is_present(pde));

	/*
	 * Write the physical address into the page table entry for the given
	 * virtual address we are mapping.
	 */
	pte = get_pte(pt, virt);
	entry_set_addr(pte, phys);
	entry_set_flags(pte, flags);

	return 0;
}

int __map_page(struct entry_table *pd, unsigned long virt,
	       struct page *page, int flags)
{
	unsigned i;

	/*
	 * We don't assume the system-wide page size (PAGE_SIZE) is the same
	 * as the page size on x86 (X86_PAGE_SIZE).
	 */
	for (i = 0; i < PAGE_SIZE / X86_PAGE_SIZE; i++) {
		unsigned long v = virt + (i * X86_PAGE_SIZE);
		unsigned long p = page_address(page) + (i * X86_PAGE_SIZE);
		int ret;

		ret = map(pd, v, p, flags);

		if (ret) {
			mmu_unmap_page(pd, virt);
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Map a page of physical memory into the virtual address space.
 *
 * @param pd The address of the page directory
 * @param virt The virtual address to map
 * @param page The physical page to map.
 * @param flags
 *
 * @return 0 on success, non-0 on error
 */
int mmu_map_page(void *pd, unsigned long virt, struct page *page, int flags)
{
	TRACE("pd=%p, virt=0x%x, page=0x%x, flags=%p",
	      pd, virt, page_address(page), flags);

	ASSERT(is_page_aligned(virt));

	return __map_page((struct entry_table *) pd, virt, page, flags);
}

/**
 * @brief This is the main page fault handling routine for arch/x86.
 * It's job is to parse the architecture generated exception and pass
 * it up to the kernel virtual memory manager to be handled.
 */
void page_fault(int vector, int error, struct registers *regs)
{
	int flags = 0;
	int ret;

	TRACE("vector=%d, error=%d, regs=%p", vector, error, regs);

	if (error & 1) flags |= PF_PRESENT;
	if (error & 2) flags |= PF_WRITE;
	else           flags |= PF_READ;
	if (error & 4) flags |= PF_USER;
	else           flags |= PF_SUPERVISOR;

	ret = vm_page_fault(regs->cr2, flags);

	if (ret) {
		//TODO: kill the process
		exn_panic(vector, error, regs);
	}
}
