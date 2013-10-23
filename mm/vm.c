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
 * 
 * We break up the user's virtual address space into regions, based on 
 * different properties we whish to assign these regions. Region flags
 * include:
 *   - Read or Write access
 *   - Priveledge level
 * A region is a contiguous range of the virtual address that have the same
 * backing store (e.g. main memory or hard disk) and the same flags.
 *
 *
 * The virtual memory system does have the concept of pages, but they do
 * not have to be the same sized pages as the backing store's pages. The
 * page size must, of course, be a multiple of the hardware page size
 * though.
 *
 * @author David Matlack
 */
#include <mm/vm.h>
#include <mm/mem.h>

#include <kernel/kmalloc.h>
#include <stddef.h>

#ifdef ARCH_X86
#include <x86/vm.h>
#endif

struct vm_region kimg_region;
struct vm_region kmem_region;

void vm_region_init(struct vm_region *r, size_t address, size_t size,
                    int flags) {
  r->flags = flags;
  r->address = address;
  r->size = size;
  r->object = NULL; //FIXME
}


int vm_init(void) {

#ifdef ARCH_X86
  x86_vm_init(PAGE_SIZE);
#endif

  vm_region_init(&kimg_region, KIMG_START, KIMG_END,
                 VM_FLAGS_SUPERVISOR | VM_FLAGS_READONLY);

  vm_region_init(&kmem_region, KMEM_START, KMEM_END,
                 VM_FLAGS_SUPERVISOR | VM_FLAGS_READWRITE);

  return 0;
}

int vm_space_init(struct vm_space *vm) {

  list_init(&vm->regions);

  return 0;
}

/**
 * @brief Add a new region to the virtual address space and return a reference
 * to the vm_region it was added to.
 *
 * @warning This region must not overlap with any existing regions in the 
 * vm_space.
  */
struct vm_region *vm_add_region(struct vm_space *vm, size_t address,
                                size_t size, int flags) {
  struct vm_region *newr = NULL;

  newr = kmalloc(sizeof(struct vm_region));
  vm_region_init(newr, address, size, flags);

  list_insert_tail(&vm->regions, newr, region_link);

  return newr;
}
