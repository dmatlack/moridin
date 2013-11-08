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

/**
 * @brief Using the provided page directory, translate the specified
 * virtual address into its backing physical address.
 */
size_t vtop(struct entry_table *pd,  size_t vaddr) {
  struct entry_table *pt;
  entry_t *pde, *pte;
  
  TRACE("pd=%p, vaddr=0x%08x", pd, vaddr);

  pde = get_pagedir_entry(pd, vaddr);

  if (!entry_is_present(pde)) { 
    DEBUG("PDE was not present!");
    return -1; //FIXME 0xffffffff
  }

  pt = (struct entry_table *) entry_get_addr(pde);
  pte = get_pagetbl_entry(pt, vaddr);

  if (!entry_is_present(pte)) { 
    DEBUG("PTE was not present!");
    return -1; //FIXME 0xffffffff
  }

  return entry_get_addr(pte) + PHYS_OFFSET(vaddr);
}
