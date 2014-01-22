/**
 * @file mm/vm.h
 *
 * @brief Virtual Memory
 */
#include <mm/memory.h>
#include <mm/pages.h>

#include <kernel/kmalloc.h>

#ifdef ARCH_X86
  #include <arch/x86/vm.h>
#endif

#include <assert.h>
#include <debug.h>
#include <errno.h>

struct vm_space boot_vm_space;

int vm_init(void) {
  boot_vm_space.object = boot_page_dir;
  return 0;
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
  int ret;
  
#ifdef ARCH_X86
  ret = x86_init_page_dir((struct entry_table **) &space->object);  
  if (ret != 0) {
    return ret;
  }
#endif

  return 0;
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
int vm_map(struct vm_space *space, size_t address, size_t size,
           vm_flags_t flags) {
  size_t *vpages;
  size_t *ppages;
  int num_pages;
  int ret = 0;
  int map_failed;
  int i;

  TRACE("space=%p, address=0x%x, size=0x%x, flags=0x%x",
        space, address, size, flags);

  address = PAGE_ALIGN_DOWN(address);
  size = PAGE_ALIGN_UP(size);
  num_pages = size / PAGE_SIZE;

  ppages = kmalloc(sizeof(size_t) * num_pages);
  if (NULL == ppages) {
    return ENOMEM;
  }
  /*
   * TODO: We can reduce the amount of malloc'ing here by changing the
   * x86 interface to map a range of addresses rather than a list
   */
  vpages = kmalloc(sizeof(size_t) * num_pages);
  if (NULL == vpages) {
    ret = ENOMEM;
    goto map_free_ppages;
  }

  for (i = 0; i < num_pages; i++) {
    vpages[i] = address + (i * PAGE_SIZE);
  }

  ret = alloc_pages(num_pages, ppages);
  if (ret != 0) goto map_free_vpages;

#ifdef ARCH_X86
  map_failed = x86_map_pages(space->object, vpages, ppages, num_pages, flags);
#endif

  if (map_failed) {
    ret = map_failed;
    free_pages(num_pages, ppages);
    goto map_free_vpages;
  }

map_free_vpages:
  kfree(vpages, sizeof(size_t) * num_pages);
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

#ifdef ARCH_X86
    x86_unmap_pages(space->object, &vpage, &ppage, 1); 
#endif

    free_pages(1, &ppage);
  }
}
