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
#include <kernel/debug.h>
#include <errno.h>

#include <arch/vm.h>

struct vm_space kernel_space; /* an address space with only the kernel mapped */
struct page *   kernel_pages; /* all the physical pages in the kernel's address space */
unsigned long   num_kernel_pages; /* the number of pages in kernel_pages */

void vm_init(void) {
  struct page *page;

  TRACE();

  list_init(&kernel_space.mappings);

  /*
   * Reserve the first 1/4 of physical memory for the kernel.
   */
  num_kernel_pages = PAGE_ALIGN_DOWN(phys_mem_bytes / 4 * 1) / PAGE_SIZE;
  kernel_pages = alloc_pages_at(0x0, num_kernel_pages);

  ASSERT_GREATEREQ(num_kernel_pages * PAGE_SIZE, MB(16));
  ASSERT_NOT_NULL(kernel_pages);

  /*
   * Create a virtual address space that only maps the kernel's address
   * space.
   */
  kernel_space.object = new_address_space();
  ASSERT_NOT_NULL(kernel_space.object);

  TRACE_OFF;

  for (page = kernel_pages; page < kernel_pages + num_kernel_pages; page++) {
    size_t phys = page_address(page);
    size_t virt = CONFIG_KERNEL_VIRTUAL_START + phys;
    int ret;

    ret = map_page(kernel_space.object, virt, page, VM_P | VM_S | VM_G | VM_R | VM_W);
    ASSERT_EQUALS(0, ret);
  }

  TRACE_ON;

  /*
   * Finally switch off the boot virtual address space and into our new,
   * virtual address space.
   */
  swap_address_space(kernel_space.object);

  /*
   * Resize the kernel heap to match to new kernel address space.
   */
  kmalloc_late_init(CONFIG_KERNEL_VIRTUAL_START + (num_kernel_pages * PAGE_SIZE));
}

int vm_space_init(struct vm_space *space) {
  /*
   * Initializing a new address space is the same as forking the address space
   * that only maps the kernel.
   */
  return vm_space_fork(space, &kernel_space);
}

int vm_space_fork(struct vm_space *to, struct vm_space *from) {
  struct vm_mapping *m;
  int error;

  to->object = new_address_space();
  if (!to->object) {
    return ENOMEM;
  }

  /*
   * Prepare the virtual memory management data structures for the fork.
   * This function is responsible for copying all the mappings between
   * from and to, and marking all pages read-only for copy-on-write.
   */
  error = fork_address_space(to->object, from->object);
  if (error) {
    goto vm_fork_fail;
  }

  /*
   * Copy the vm_mappings between each.
   */
  error = ENOMEM;
  list_init(&to->mappings);
  list_foreach(m, &from->mappings, link) {
    struct vm_mapping *new_m;

    new_m = kmalloc(sizeof(struct vm_mapping));
    if (!new_m) {
      goto vm_fork_fail;
    }

    memcpy(new_m, m, sizeof(struct vm_mapping));
    new_m->space = to;
    atomic_add(&new_m->file->refs, 1);

    list_insert_tail(&to->mappings, new_m, link);
  }

  return 0;

vm_fork_fail:
  vm_space_destroy(to);
  return error;
}

void vm_space_destroy(struct vm_space *space) {
  (void)space; panic("implement me");
}

int vm_map_page(struct vm_space *space, unsigned long virt, int flags) {
  struct page *page;
  int error;

  page = alloc_page();
  if (!page) {
    return ENOMEM;
  }

  error = map_page(space->object, virt, page, flags);
  if (error) {
    free_page(page);
    return ENOMEM;
  }

  tlb_invalidate(virt, PAGE_SIZE);

  return 0;
}

void vm_unmap_page(struct vm_space *space, unsigned long virt) {
  struct page *page;

  page = unmap_page(space->object, virt);
  if (page) {
    free_page(page);
    tlb_invalidate(virt, PAGE_SIZE);
  }
}

void vm_dump_maps(printf_f p, struct vm_space *space) {
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
