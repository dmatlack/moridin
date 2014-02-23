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

#ifdef ARCH_X86
  #include <arch/x86/vm.h>
#endif

struct vm_space *postboot_vm_space;

void vm_init(void) {
  unsigned i, nkpages;
  size_t ksize;
  int ret;

  TRACE();

  /*
   * Reserve the first 1/4 of physical memory
   */
  ksize = PAGE_ALIGN_DOWN(phys_mem_bytes / 4 * 1);
  ASSERT_GREATEREQ(ksize, MB(16));
  alloc_kernel_pages(0x0, ksize);

  postboot_vm_space = kmalloc(sizeof(struct vm_space));
  if (NULL == postboot_vm_space) {
    panic("Couldn't allocate the posboot_vm_space struct.");
  }
  ret = vm_space_init(postboot_vm_space);
  if (ret) {
    panic("Couldn't initialize postboot_vm_space: %d/%s", ret, strerr(ret));
  }

  TRACE_OFF; // about to map a lot of pages, so disable debug call tracing

  nkpages = ksize / PAGE_SIZE;
  for (i = 0; i < nkpages; i++) {
    size_t paddr = 0x0 + i*PAGE_SIZE;
    size_t vaddr = CONFIG_KERNEL_VIRTUAL_START + i*PAGE_SIZE;

    ret = map_pages(postboot_vm_space->object, vaddr, PAGE_SIZE, &paddr,
        VM_S | VM_G | VM_R | VM_W);
    if (ret) {
      panic("Couldn't map kernel address space: %d/%s", ret, strerr(ret));
    }
  }

  TRACE_ON;

  /*
   * Finally switch off the boot virtual address space and into our new,
   * virtual address space.
   */
  __vm_space_switch(postboot_vm_space->object);

  /*
   * Resize the kernel heap to match to new kernel address space.
   */
  kmalloc_late_init(CONFIG_KERNEL_VIRTUAL_START + ksize);
}

/**
 * @brief Initialize a virtual address space.
 *
 * This function does not enable paging and change the current
 * virtual address space.
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

int __vm_map(struct vm_space *space, size_t address, size_t size, size_t *ppages, int flags) {
  int ret;

  ret = map_pages(space->object, address, size, ppages, flags);

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
  size_t *ppages;
  int num_pages;
  int ret = 0;
  int map_failed;

  TRACE("space=%p, address=0x%x, size=0x%x, flags=0x%x",
        space, address, size, flags);

  address = PAGE_ALIGN_DOWN(address);
  size = PAGE_ALIGN_UP(size);
  num_pages = size / PAGE_SIZE;

  ppages = kmalloc(sizeof(size_t) * num_pages);
  if (NULL == ppages) {
    return ENOMEM;
  }

  ret = alloc_pages(num_pages, ppages);
  if (ret != 0) goto map_free_ppages;

  map_failed = __vm_map(space, address, size, ppages, flags);

  if (map_failed) {
    ret = map_failed;
    free_pages(num_pages, ppages);
    goto map_free_ppages;
  }

map_free_ppages:
  kfree(ppages, sizeof(size_t) * num_pages);
  return ret;
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

