/**
 * @file x86/vm.c
 *
 * @brief x86 Virtual Memory Implementation
 *
 * @author David Matlack
 */
#include <x86/vm.h>
#include <x86/page.h>

#include <stddef.h>
#include <assert.h>

//TODO provide an implementation of vm_object that the kernel level virtual
// memory manager can use to fulfill pages of memory.

int x86_vm_init(size_t kernel_page_size) {

  // WE ONLY SUPPORT 4 KB PAGES
  assert(X86_PAGE_SIZE == KB(4));
  assert(kernel_page_size / X86_PAGE_SIZE * X86_PAGE_SIZE == kernel_page_size);

  return 0;
}

inline int la_to_pde(int la) {
  return (la >> 22) & 1023;
}
