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
#include <debug.h>

#include <kernel/kmalloc.h>
#include <stddef.h>

#ifdef ARCH_X86
#include <x86/vm.h>
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
  TRACE("void");

  VM_ZONE_KERNEL->size = PMEM_ZONE_KERNEL->size;
  VM_ZONE_KERNEL->address = (size_t) (0 - VM_ZONE_KERNEL->size);
  /* 
   * If the kernel is mapped to the top of the address space (which it 
   * probably is), then steal the top page from it, otherwise we run into 
   * overflow issues (FIXME hack)
   */
  if (0 == VM_ZONE_KERNEL->address + VM_ZONE_KERNEL->size) {
    VM_ZONE_KERNEL->size -= PAGE_SIZE;
  }
  LOG_VM_ZONE(VM_ZONE_KERNEL);

  VM_ZONE_USER->address = MB(16);
  VM_ZONE_USER->size = VM_ZONE_KERNEL->address - VM_ZONE_USER->address;
  LOG_VM_ZONE(VM_ZONE_USER);

  if (0 != machine->bootstrap(PAGE_SIZE)) {
    return -1;
  }

  return 0;
}
