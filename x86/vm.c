/**
 * @file x86/vm.c
 *
 * @brief x86 Virtual Memory Implementation
 *
 * @author David Matlack
 */
#include <x86/vm.h>
#include <x86/page.h>
#include <x86/reg.h>

#include <mm/physmem.h>
#include <kernel/kmalloc.h>

#include <stddef.h>
#include <assert.h>

x86_entry_t __initial_entry;

int x86_vm_init(size_t kernel_page_size) {
  int e;

  // WE ONLY SUPPORT 4 KB PAGES
  assert(sizeof(int) == 4);
  assert(X86_PAGE_SIZE == KB(4));
  assert(kernel_page_size / X86_PAGE_SIZE * X86_PAGE_SIZE == 
         kernel_page_size);

  /*
   * The default setting of flags for page directory/table entries.
   */
  e = 0;
  SET_BIT(e, PDE_PRESENT,   0); // the entry is not present
  SET_BIT(e, PDE_PCD,       1); // disable caching
  SET_BIT(e, PDE_PS,        0); // pages are 4KB
  __initial_entry = e;

  return 0;
}

/**
 * @brief Allocate an x86_pgdir struct.
 */
struct x86_pgdir *x86_pgdir_alloc(void) {
  return kmemalign(X86_PAGE_SIZE, sizeof(struct x86_pgdir));
}

/**
 * @brief Free an x86_pdir struct.
 */
void x86_pgdir_free(struct x86_pgdir *pd) {
  kfree(pd, sizeof(struct x86_pgdir));
}

/**
 * @brief Initialize a page directory by setting all default flags and marking
 * each entry as NOT PRESENT.
 */
int x86_pgdir_init(struct x86_pgdir *pd) {
  int i;
  for (i = 0; i < X86_PD_SIZE; i++) {
    pd->entries[i] = __initial_entry;
  }
  return 0;
}
