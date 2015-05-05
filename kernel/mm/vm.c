/**
 * @file mm/vm.h
 *
 * @brief Virtual Memory
 */
#include <mm/memory.h>
#include <mm/pages.h>

#include <kernel/config.h>
#include <mm/kmalloc.h>

#include <assert.h>
#include <errno.h>

#include <arch/vm.h>
#include <arch/fork.h>

struct vm_space kernel_space;       /* an address space with only the kernel mapped */
struct page *   kdirect_pages;      /* all the physical pages in the kernel's address space */
unsigned long   kdirect_num_pages;  /* the number of pages in the kdirect region */

void vm_init(void)
{
	struct page *page;

	TRACE();

	/*
	 * Create a virtual address space that only maps the kernel's virtual
	 * address space.
	 */
	kernel_space.mmu = new_address_space();
	ASSERT_NOT_NULL(kernel_space.mmu);
	list_init(&kernel_space.mappings);

	/*
	 * Direct map the kdirect region of virtual memory.
	 */
	kdirect_num_pages = ((size_t) kdirect_end - (size_t) kdirect_start) /
			    PAGE_SIZE;
	kdirect_pages = alloc_pages_at(0x0, kdirect_num_pages);
	ASSERT_NOT_NULL(kdirect_pages);

	for (page = kdirect_pages; page < kdirect_pages + kdirect_num_pages;
	     page++) {
		size_t virt = (size_t) kdirect_start + page_address(page);
		int ret;

		ret = mmu_map_page(kernel_space.mmu, virt, page,
				   VM_P | VM_S | VM_G | VM_R | VM_W);
		ASSERT_EQUALS(0, ret);
	}

	/*
	 * Finally switch off the boot virtual address space and into our new,
	 * virtual address space.
	 */
	swap_address_space(kernel_space.mmu);

	/*
	 * Resize the kernel heap to match to new kernel address space.
	 */
	kmalloc_late_init();
}

int vm_space_init(struct vm_space *space)
{
	/*
	 * Initializing a new address space is the same as forking the address
	 * space that only maps the kernel.
	 */
	return vm_space_fork(space, &kernel_space);
}

struct vm_mapping *new_vm_mapping(unsigned long addr, unsigned long length,
				  int vmflags, struct vfs_file *file,
				  unsigned long off)
{
	struct vm_mapping *m;

	m = kmalloc(sizeof(*m));
	if (!m)
		return NULL;

	m->address = addr;
	m->num_pages = length / PAGE_SIZE;
	m->flags = vmflags;
	m->file = file;
	m->foff = off;
	cond_vfs_file_get(file);

	return m;
}

void free_vm_mapping(struct vm_mapping *m)
{
	cond_vfs_file_put(m->file);
	kfree(m, sizeof(*m));
}

static struct vm_mapping *vm_mapping_fork(struct vm_mapping *from)
{
	return new_vm_mapping(
		from->address, from->num_pages * PAGE_SIZE, from->flags,
		from->file, from->foff);
}

int vm_space_fork(struct vm_space *to, struct vm_space *from)
{
	struct vm_mapping *m;
	int error;

	to->mmu = new_address_space();
	if (!to->mmu) {
		return ENOMEM;
	}

	/*
	 * Prepare the virtual memory management data structures for the fork.
	 * This function is responsible for copying all the mappings between
	 * from and to, and marking all pages read-only for copy-on-write.
	 */
	error = fork_address_space(to->mmu, from->mmu);
	if (error) {
		goto vm_fork_fail;
	}

	/*
	 * Copy the vm_mappings between each.
	 */
	error = ENOMEM;
	list_init(&to->mappings);
	list_foreach(m, &from->mappings, link) {
		struct vm_mapping *copy = vm_mapping_fork(m);

		if (!copy)
			goto vm_fork_fail;

		copy->space = to;
		list_insert_tail(&to->mappings, copy, link);
	}

	return 0;

vm_fork_fail:
	vm_space_destroy(to);
	return error;
}

void vm_space_destroy(struct vm_space *space)
{
	/*
	 * Switch to the kernel-only address space so we don't have to worry
	 * about destroying our own address space.
	 */
	swap_address_space(kernel_space.mmu);

	while (!list_empty(&space->mappings)) {
		struct vm_mapping *m = list_dequeue(&space->mappings, link);

		/* frees the mapping */
		__vm_munmap(space, m->address, M_LENGTH(m));
	}

	free_address_space(space->mmu);
}

int vm_map_page(struct vm_space *space, unsigned long virt, int flags)
{
	struct page *page;
	int error;

	page = alloc_page();
	if (!page) {
		return ENOMEM;
	}

	error = mmu_map_page(space->mmu, virt, page, flags);
	if (error) {
		free_page(page);
		return ENOMEM;
	}

	tlb_invalidate(virt, PAGE_SIZE);

	return 0;
}

void vm_unmap_page(struct vm_space *space, unsigned long virt)
{
	struct page *page;

	page = mmu_unmap_page(space->mmu, virt);
	if (page) {
		free_page(page);
		tlb_invalidate(virt, PAGE_SIZE);
	}
}

void vm_dump_maps(printf_f p, struct vm_space *space)
{
	struct vm_mapping *m;

	TRACE("space=%p", space);

	list_foreach(m, &space->mappings, link) {
		p("0x%08x - 0x%08x %c%c%c%c%c%c%c",
				m->address, M_END(m),
				m->flags & VM_R ? 'r' : '-',
				m->flags & VM_W ? 'w' : '-',
				m->flags & VM_X ? 'x' : '-',
				m->flags & VM_U ? 'u' : '-',
				m->flags & VM_S ? 's' : '-',
				m->flags & VM_G ? 'g' : '-',
				m->flags & VM_P ? 'p' : '-');
		if (m->file) {
			p(" %s 0x%x", m->file->dirent->name, m->foff);
		}
		p("\n");
	}
}
