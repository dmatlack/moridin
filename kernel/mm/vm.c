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

#define KERNEL_VIRTUAL_START 0x00000000

struct vm_space postboot_vm_space;

int vm_init(void) {
  unsigned i, nkpages;
  size_t ksize;

  /*
   * Reserve the first 1/4 of physical memory
   */
  ksize = PAGE_ALIGN_DOWN(phys_mem_bytes / 4 * 3);
  ASSERT_GREATEREQ(ksize, MB(16));
  reserve_kernel_pages(0x0, ksize);

  /*
   * Map the reserved pages into a new address space.
   */
  vm_space_init(&postboot_vm_space);

  TRACE_OFF; // about to map a lot of pages, so disable debug call tracing

  nkpages = num_kernel_pages();
  for (i = 0; i < nkpages; i++) {
    size_t paddr = kernel_pages_pstart() + i*PAGE_SIZE;
    size_t vaddr = KERNEL_VIRTUAL_START + i*PAGE_SIZE;
    __vm_map(&postboot_vm_space, vaddr, PAGE_SIZE, &paddr, VM_S | VM_R | VM_W); 
  }

  TRACE_ON;

  /*
   * Finally switch off the boot virtual address space and into our new,
   * "real" virtual address space.
   */
  vm_space_switch(&postboot_vm_space);

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

void vm_space_switch(struct vm_space *space) {
#ifdef ARCH_X86
  x86_set_pagedir(space->object);
#endif
}

int __vm_map(struct vm_space *space, size_t address, size_t size,
             size_t *ppages, vm_flags_t flags) {
  int ret;

#ifdef ARCH_X86
  ret = x86_map_pages(space->object, address, size, ppages, flags);
#endif

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
int vm_map(struct vm_space *space, size_t address, size_t size,
           vm_flags_t flags) {
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

#ifdef ARCH_X86
    x86_unmap_pages(space->object, vpage, PAGE_SIZE,  &ppage); 
#endif

    free_pages(1, &ppage);
  }
}
