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
#include <x86/cpu.h>

#include <mm/physmem.h>
#include <mm/vm.h>
#include <kernel/kmalloc.h>

#include <stddef.h>
#include <assert.h>
#include <debug.h>

#define CURRENT_PAGE_DIR ((struct entry_table *) get_cr3())

#define IS_PAGE_ALIGNED(addr)\
  (FLOOR(X86_PAGE_SIZE, addr) == addr)

int entry_table_init(struct entry_table *tbl) {
  unsigned int i;
  for (i = 0; i < ENTRY_TABLE_SIZE; i++) {
    /*
     * zero out the entry to be safe but all that matters here is that the
     * page entry is marked as not present.
     */
    tbl->entries[i] = 0;
    entry_set_absent(tbl->entries + i);
  }
  return 0;
}

struct entry_table *alloc_entry_table(void) {
  return kmemalign(X86_PAGE_SIZE, sizeof(struct entry_table));
}

int __x86_map_page(struct entry_table *pd, size_t vpage, size_t ppage) {
  struct entry_table *pt;
  entry_t *pde, *pte;

  ASSERT(IS_PAGE_ALIGNED(vpage));
  ASSERT(IS_PAGE_ALIGNED(ppage));

  pde = get_pde(pd, vpage);

  if (!entry_is_present(pde)) {
    pt = alloc_entry_table();

    if (NULL == pt) {
      return ALLOC_FAILED;
    }
    else {
      entry_table_init(pt);
      entry_set_present(pde);
      entry_set_addr(pde, (size_t) pt);
    }
  }

  ASSERT(entry_is_present(pde));

  pte = get_pte(pt, vpage);
  entry_set_present(pte);
  entry_set_addr(pte, ppage);

  ASSERT(ppage == __x86_vtop(pd, vpage));

  return 0;
}

/**
 * @brief Maps the virtual page, <vpage>, to the physical page, <ppage>.
 *
 * @return 0 on success, non-0 if the mapping could not be completed.
 *  Failure Scenarios:
 *    - Mapping this page requires allocating a new page table data
 *     structure and the system has run out of available memory.
 */
int x86_map_page(size_t vpage, size_t ppage) {
  return __x86_map_page(CURRENT_PAGE_DIR, vpage, ppage);
}

size_t __x86_vtop(struct entry_table *pd,  size_t vaddr) {
  struct entry_table *pt;
  entry_t *pde, *pte;
  
  pde = get_pde(pd, vaddr);

  if (!entry_is_present(pde)) { 
    DEBUG("PDE was not present!");
    return -1; //FIXME 0xffffffff
  }

  pt = (struct entry_table *) entry_get_addr(pde);
  pte = get_pte(pt, vaddr);

  if (!entry_is_present(pte)) { 
    DEBUG("PTE was not present!");
    return -1; //FIXME 0xffffffff
  }

  return entry_get_addr(pte) + PHYS_OFFSET(vaddr);
}

/**
 * @brief Convert the virtual address into the physical address it is
 * mapped to in the current address space.
 */
size_t x86_vtop(size_t vaddr) {
  return __x86_vtop(CURRENT_PAGE_DIR, vaddr);
}
