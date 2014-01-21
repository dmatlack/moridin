/**
 * @file x86/vm.c
 *
 * @brief x86 Virtual Memory Implementation
 *
 * @author David Matlack
 */
#include <arch/x86/vm.h>
#include <arch/x86/page.h>
#include <arch/x86/reg.h>
#include <arch/x86/cpu.h>

#include <kernel/kmalloc.h>

#include <mm/memory.h>
#include <mm/vm.h>

#include <stddef.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>

#define X86_IS_PAGE_ALIGNED(addr)\
  (FLOOR(X86_PAGE_SIZE, addr) == addr)

int x86_init_page_dir(struct entry_table **object) {
  struct entry_table *page_directory;
  int ret;

  page_directory = entry_table_alloc();
  if (NULL == page_directory) return ENOMEM;

  ret = entry_table_init(page_directory);
  if (0 != ret) {
    entry_table_free(page_directory); 
    return ret;
  }

  *object = page_directory;
  return 0;
}

/**
 * @brief Initialize a page table or page directory (struct entry_table).
 *
 * This sets all entries in the table to 0. Most importantly, it marks all
 * entries as *not* present.
 */
int entry_table_init(struct entry_table *tbl) {
  unsigned int i;
  TRACE("tbl=%p", tbl);
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
 * @brief Allocate a new entry_table, with the proper memory address alignment.
 *
 * @return NULL if the allocation fails.
 */
struct entry_table *entry_table_alloc(void) {
  TRACE();
  return kmemalign(X86_PAGE_SIZE, sizeof(struct entry_table));
}

void entry_table_free(struct entry_table *ptr) {
  TRACE("ptr=%p", ptr);
  kfree(ptr, sizeof(struct entry_table));
}

/**
 * @brief Set the entry flags to match the provided vm_flags_t.
 */
void entry_set_flags(entry_t *entry, vm_flags_t flags) {
  if (flags & VM_W) {
    entry_set_readwrite(entry);
  }
  else {
    entry_set_readonly(entry);
  }

  if (flags & VM_S) {
    entry_set_supervisor(entry);
  }
  else {
    entry_set_user(entry);
  }
}

/**
 * @brief Maps the virtual page, <vpage>, to the physical page, <ppage>.
 *
 * @return
 *    0 on success, non-0 if the mapping could not be completed.
 *
 *    ENOMEM if mapping this page requires allocating a new page
 *      table data structure and the system has run out of available
 *      memory.
 */
int x86_map_page(struct entry_table *pd, size_t vpage, size_t ppage,
                 vm_flags_t flags) {
  struct entry_table *pt;
  size_t vtop;
  entry_t *pde, *pte;

  TRACE("pd=%p, vpage=0x%x, ppage=0x%x, flags=0x%x", pd, vpage, ppage, flags);
  ASSERT(X86_IS_PAGE_ALIGNED(vpage));
  ASSERT(X86_IS_PAGE_ALIGNED(ppage));

  pde = get_pde(pd, vpage);

  if (!entry_is_present(pde)) {
    pt = entry_table_alloc();

    if (NULL == pt) {
      return ENOMEM;
    }
    else {
      entry_table_init(pt);

      entry_set_addr(pde, (size_t) pt);
      entry_set_present(pde);
      entry_set_flags(pde, VM_R | VM_W | VM_U);
    }
  }

  ASSERT(entry_is_present(pde));

  pte = get_pte(pt, vpage);

  entry_set_addr(pte, ppage);
  entry_set_present(pte);
  entry_set_flags(pte, flags);

  ASSERT(x86_vtop(pd, vpage, &vtop));
  ASSERT_EQUALS(ppage, vtop);

  return 0;
}

/**
 * @brief Unmap the set of pages.
 *
 * This function only unmaps the pages in the x86 virtual memory
 * data structures. It does not decrease the reference counter to the
 * unmapped pages or "free" them in any way. That is up to the caller.
 *
 * @param pd The page directory
 * @param vpages The virtual addresses of the pages to unmap
 * @param ppages The physical addresses of the pages that were unmapped
 * @param num_pages The number of pages to unmap
 */
void x86_unmap_pages(struct entry_table *pd, size_t *vpages,
                     size_t *ppages, int num_pages) {
  size_t vpage;
  entry_t *pde, *pte;
  int i, j;

  TRACE("pd=%p, vpages=%p, ppages=%p, num_pages=%d",
        pd, vpages, ppages, num_pages);

  for (i = 0; i < num_pages; i++) {
    vpage = vpages[i];

    /*
     * Mark the page _directory_ entry so we know that we unmapped a page
     * in this page table.
     */
    pde = get_pde(pd, vpage);
    *pde = *pde | ENTRY_TABLE_UNMAP;

    /*
     * Mark the page _table_ entry as not present, effectively unmapping
     * the page.
     */
    pte = get_pte((struct entry_table *) entry_get_addr(pde), vpage);
    entry_set_absent(pte);

    ppages[i] = entry_get_addr(pte);
  }

  for (i = 0; i < (int) ENTRY_TABLE_SIZE; i++) {
    pde = pd->entries + i;

    /*
     * If we marked the page _directory_ entry, then we need to check 
     * if the corresponding page table is empty, and if so free it.
     */
    if (*pde & ENTRY_TABLE_UNMAP) {
      struct entry_table *pt = (struct entry_table *) entry_get_addr(pde);
      bool is_empty = true;

      for (j = 0; j < (int) ENTRY_TABLE_SIZE; j++) {
        pte = pt->entries + j;
        if (entry_is_present(pte)) {
          is_empty = false;
          break;
        }
      }

      /*
       * If the page table was empty, mark the page directory entry as
       * absent and free the page table.
       */
      if (is_empty) {
        entry_set_absent(pde);
        entry_table_free(pt);
      }

      *pde &= ~ENTRY_TABLE_UNMAP;
    }
  }
}

/**
 * @brief Map the <num_pages> virtual pages specified in <vpages> to the
 * <num_pages> physical pages specified in <ppages>, all with the provided
 * flags.
 *
 * @warning vpages and ppages must both be arrays of at least length
 * <num_pages>.
 *
 * @return 0 on success, non-0 on error
 */
int x86_map_pages(struct entry_table *pd, size_t *vpages, size_t *ppages, 
                  int num_pages, vm_flags_t flags) {
  int i;

  TRACE("pd=%p, vpages=%p, ppages=%p, num_pages=%d, flags=0x%x",
        pd, vpages, ppages, num_pages, flags);
  for (i = 0; i < num_pages; i++) {
    int ret = x86_map_page(pd, vpages[i], ppages[i], flags);

    if (0 != ret) {
      x86_unmap_pages(pd, vpages, ppages, i);
      return ret;
    }
  }

  return 0;
}

/**
 * @brief Convert the virtual address into the physical address it is
 * mapped to.
 *
 * @return TRUE (non-zero) if the address is mapped, FALSE (zero) if the
 * address is not mapped.
 */
bool x86_vtop(struct entry_table *pd,  size_t vaddr, size_t *paddrp) {
  struct entry_table *pt;
  entry_t *pde, *pte;

  ASSERT_NOT_NULL(paddrp);
  
  pde = get_pde(pd, vaddr);

  if (!entry_is_present(pde)) { 
    return false;
  }

  pt = (struct entry_table *) entry_get_addr(pde);
  pte = get_pte(pt, vaddr);

  if (!entry_is_present(pte)) { 
    return false;
  }

  *paddrp = entry_get_addr(pte) + PHYS_OFFSET(vaddr);
  return true;
}
