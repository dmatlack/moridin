/**
 * @file mm/vm.h
 *
 * @brief Virtual Memory
 */
#include <mm/memory.h>
#include <mm/pages.h>

#include <kernel/config.h>
#include <kernel/kmalloc.h>

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

    ret = map_page(kernel_space.object, virt, page, VM_S | VM_G | VM_R | VM_W);
    ASSERT_EQUALS(0, ret);
  }

  TRACE_ON;

  /*
   * Finally switch off the boot virtual address space and into our new,
   * virtual address space.
   */
  __vm_space_switch(kernel_space.object);

  /*
   * Resize the kernel heap to match to new kernel address space.
   */
  kmalloc_late_init(CONFIG_KERNEL_VIRTUAL_START + (num_kernel_pages * PAGE_SIZE));
}

/**
 * @brief Initialize a virtual address space.
 *
 * This function does not enable paging or change the current virtual address
 * space.
 *
 * @return
 *    0 on success
 *    non-0 on error (architecture-dependent)
 */
int vm_space_init(struct vm_space *space) {
  space->object = new_address_space();
  if (!space->object) {
    return ENOMEM;
  }

  /*
   * Map the kernel into the new address space.
   */
  share_mappings(space->object, kernel_space.object);

  return 0;
}

void vm_space_destroy(struct vm_space *space) {
  (void)space;
  panic("Implement me: %s", __func__);
}

/**
 * @brief Switch address spaces.
 *
 * @warning This function does not allow you to get the vm_space struct of
 * old object. So this function is not safe to use if you care about locking
 * the vm_space that is currently in use and have no other way of accessing
 * it.
 *
 * @param object This should be a (struct vm_space *)->object, or the object
 * returned by a previous invocation to this function.
 *
 * @return The old object.
 */
void *__vm_space_switch(void *object) {
  return swap_address_space(object);
}

int __vm_map(struct vm_space *space, size_t address, struct page *pages, unsigned num_pages, int flags) {
  struct page *page;
  int ret;
  
  for (page = pages; page < pages + num_pages; page++) {
    size_t virt = address + (page_address(page) - page_address(pages));

    ret = map_page(space->object, virt, page, flags);

    //FIXME
    ASSERT_EQUALS(0, ret);
  }

  return ret;
}


/**
 * @brief Map a range of addresses into the virtual address space.
 *
 * Note that this function is all-or-nothing. Either the entire region
 * gets mapped, or an error occurs and none of the region is mapped.
 *
 * @warning <address> will be aligned down to the nearest page and
 * <size> will be aligned up to the nearest page.
 *
 * @return
 *    0 on success
 *    ENOMEM
 *    other non-0 on architecture-dependent errors
 */
int vm_map(struct vm_space *space, size_t address, size_t size, int flags) {
  struct page *pages;
  int num_pages;
  int ret = 0;

  TRACE("space=%p, address=0x%x, size=0x%x, flags=0x%x", space, address, size, flags);

  address = PAGE_ALIGN_DOWN(address);
  size = PAGE_ALIGN_UP(size);
  num_pages = size / PAGE_SIZE;

  pages = alloc_pages(num_pages);
  if (!pages) {
    return ENOMEM;
  }

  ret = __vm_map(space, address, pages, num_pages, flags);
  if (ret) {
    NEW_free_pages(pages, num_pages);
    return ret;
  }

  return 0;
}

void vm_unmap(struct vm_space *space, size_t address, size_t size) {
  int num_pages;
  int i;

  TRACE("space=%p, address=0x%x, size=0x%x", space, address, size);

  address = PAGE_ALIGN_DOWN(address);
  size = PAGE_ALIGN_UP(size);
  num_pages = size / PAGE_SIZE;

  for (i = 0; i < num_pages; i++) {
    size_t vpage = address + (i * PAGE_SIZE);
    size_t ppage;

    unmap_pages(space->object, vpage, PAGE_SIZE,  &ppage); 
    free_pages(1, &ppage);
  }
}
