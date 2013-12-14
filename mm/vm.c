/**
 * @file mm/vm.c
 *
 * @breif The virtual memory manager for the kernel.
 *
 * This file takes a high-level, machine-indepentent view of virtual
 * memory. Virtual Memory is the concept that we take the entire address
 * space and map each page to some physical page (in memory, on a hard
 * disk, maybe even on pages stored over the network).
 *
 * A key part of virtual memory is that we do not provide a mapping for
 * every page up front (otherwise, there would only be enough memory
 * for one virtual address space in main memory at a time). Instead,
 * we provide mappings for pages as they are used by the process.
 *
 * This "demand-paging" scheme is facilitated by the page fault mechanism.
 * When a process attempts to read-from or write-to a page that does not
 * have a corresponding virtual memory mapping, a page fault exception
 * occurs. We write a routine that handles a page fault by providing a
 * page to back the requested address and resuming execution in the process.
 *
 * @author David Matlack
 */
#include <mm/vm.h>
#include <mm/physmem.h>
#include <errno.h>
#include <debug.h>

#include <kernel/kmalloc.h>
#include <stddef.h>
#include <math.h>

#ifdef ARCH_X86
#include <arch/x86/vm.h>
#endif

struct vm_zone __vm_zone_kernel;
struct vm_zone __vm_zone_user;

/*
 * struct vm_machine_interface machine
 *  
 *  This is the interface to the machine dependent virtual memory
 *  implementation. It is determined at compile-time.
 */ 
#ifdef ARCH_X86
struct vm_machine_interface *machine = &x86_vm_machine_interface;
#endif

#define LOG_VM_ZONE( zone_macro )\
  INFO(#zone_macro": address=0x%0x, size=0x%08x",\
       zone_macro->address, zone_macro->size)

/**
 * @brief Initialize the virtual memory zone structs, call the machine 
 * dependent vm bootstrap code, and return.
 *
 * After this function returns successfully, virtual memory is enabled
 * and the VM_ZONE_KERNEL will be direct mapped to PMEM_ZONE_KERNEL with
 * pages marked as read/write and PL 0.
 *
 * @return 0 on success, non-0 on error
 */
int vm_bootstrap(void) {
  int ret;

  TRACE("void");

  /*
   * Map the kernel into the lower half of the address space and the user into
   * the higher half of the address space.
   */
  VM_ZONE_KERNEL->address  = 0;
  VM_ZONE_KERNEL->size     = CONFIG_KERNEL_VM_SIZE;
  VM_ZONE_USER->address    = CONFIG_KERNEL_VM_SIZE;
  VM_ZONE_USER->size       = (size_t) 0 - PAGE_SIZE - VM_ZONE_USER->size;

  LOG_VM_ZONE(VM_ZONE_KERNEL);
  LOG_VM_ZONE(VM_ZONE_USER);

  if (0 != (ret = machine->bootstrap(PAGE_SIZE))) {
    return ret;
  }

  return 0;
}

int vm_address_space_init(struct vm_address_space *vm) {
  list_init(&vm->region_list);
  
  if (!machine->init_object(&vm->object)) {
    return -1;
  }

  return 0;
}

int vm_address_space_create(struct vm_address_space **vmp) {
  *vmp = kmalloc(sizeof(struct vm_address_space));
  if (NULL == *vmp) {
    return -1;
  }
  return vm_address_space_init(*vmp);
}

/**
 * @brief Map the provided region into the given address space.
 *
 * If this function returns successfully, then the provided region _will_ be
 * addressable (mapped) within the given address space.
 */
int vm_map_region(struct vm_address_space *vm, struct vm_region *new_region) {
  struct vm_region *r;
  size_t num_pages;
  bool inserted = false;
  int ret = 0;
  unsigned int i;

  ASSERT(NULL != vm);
  ASSERT(NULL != new_region);
  ASSERT(FLOOR(PAGE_SIZE, new_region->size) == new_region->size);

  num_pages = new_region->size / PAGE_SIZE;

  /*
   * Added the region list to the vm_address_space struct
   */
  list_foreach(r, &vm->region_list, link) {
    ASSERT(!check_overlap(r->address, r->size,
                          new_region->address, new_region->size));
    if (new_region->address < r->address) {
      list_insert_before(&vm->region_list, r, new_region, link);
      inserted = true;
    }
  }
  if (!inserted) {
    list_insert_tail(&vm->region_list, new_region, link);
  }

  /*
   * Map the pages into the machine dependent data structures.
   */
  for (i = 0; i < num_pages; i++) {
    size_t vpage = new_region->address + (i * PAGE_SIZE);
    size_t ppage;
    
    if (0 != (ret = pmem_alloc(&ppage, 1, PMEM_ZONE_USER))) {
      goto vm_region_map_cleanup;
    }

    if (0 != (ret = machine->map(vm->object, &vpage, &ppage, 1, 
                                 new_region->flags))) {
      goto vm_region_map_cleanup;
    }
  }

vm_region_map_cleanup:
  //FIXME rollback
  return ret;
}
