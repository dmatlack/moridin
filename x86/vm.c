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

int x86_vm_init(void) {

  // WE ONLY SUPPORT 4 KB PAGES
  assert(X86_PAGE_SIZE == KB(4));

  return 0;
}

inline int la_to_pde(int la) {
  return (la >> 22) & 1023;
}
