/**
 * @file x86/vm.c
 *
 * @brief x86 Virtual Memory Implementation
 *
 * @author David Matlack
 */
#include <arch/vm.h>
#include <arch/page.h>
#include <arch/reg.h>
#include <arch/cpu.h>

#include <kernel/kmalloc.h>

#include <mm/memory.h>
#include <mm/vm.h>
#include <mm/pages.h>

#include <stddef.h>
#include <assert.h>
#include <kernel/debug.h>
#include <errno.h>

static inline bool is_page_aligned(size_t addr) {
  return FLOOR(X86_PAGE_SIZE, addr) == addr;
}

/**
 * @brief Allocate a new entry_table, with the proper memory address alignment.
 *
 * @return NULL if the allocation fails.
 */
static struct entry_table *new_entry_table(void) {
  struct entry_table *tbl;
  unsigned i;

  TRACE();

  tbl = kmemalign(X86_PAGE_SIZE, sizeof(struct entry_table));

  for (i = 0; i < ENTRY_TABLE_SIZE; i++) {
    /*
     * zero out the entry to be safe but all that matters here is that the
     * page entry is marked as not present.
     */
    tbl->entries[i] = 0;
    entry_set_absent(tbl->entries + i);
  }

  return tbl;
}

static void free_entry_table(struct entry_table *ptr) {
  TRACE("ptr=%p", ptr);
  kfree(ptr, sizeof(struct entry_table));
}

/**
 * @brief Allocate a page directory to be used in a new address space.
 *
 * @return NULL if allocating a page of memory failed, the address of the 
 * page directory otherwise.
 */
void *new_address_space(void) {
  struct entry_table *page_directory;

  page_directory = new_entry_table();
  if (!page_directory) {
    return NULL;
  }

  return (void *) page_directory;
}

/**
 * @brief share_mappings() is expected to make a high level copy of the from_pd
 * address space to to_pd. This means that after this function is called, to_pd
 * should have the exact same mappings as from_pd.
 */
void share_mappings(struct entry_table *to_pd, struct entry_table *from_pd) {
  memcpy(to_pd, from_pd, sizeof(struct entry_table));
}

/**
 * @brief This function looks at the flags (see mm/vm.h) and sets the
 * correct bits in the page (table|directory) entry.
 */
void entry_set_flags(entry_t *entry, int flags) {
  if (flags & VM_W) {
    entry_set_readwrite(entry);
    TRACE("RW");
  }
  else {
    entry_set_readonly(entry);
    TRACE("RO");
  }

  if (flags & VM_S) {
    TRACE("SUPERVISOR");
    entry_set_supervisor(entry);
  }
  else {
    TRACE("USER");
    entry_set_user(entry);
  }
}

/**
 * @brief Convert the virtual address into the physical address it is
 * mapped to.
 *
 * @param pp If not NULL, the physical address will be written here.
 *
 * @return TRUE (non-zero) if the address is mapped, FALSE (zero) if the
 * address is not mapped.
 */
bool __vtop(struct entry_table *pd, size_t v, size_t *pp) {
  struct entry_table *pt;
  entry_t *pde, *pte;

  pde = get_pde(pd, v);

  if (!entry_is_present(pde)) { 
    return false;
  }

  pt = (struct entry_table *) entry_get_addr(pde);
  pte = get_pte(pt, v);

  if (!entry_is_present(pte)) { 
    return false;
  }

  if (pp) {
    *pp = entry_get_addr(pte) + PHYS_OFFSET(v);
  }

  return true;
}

/**
 * @brief Free the empty page tables referenced by the page directory. Only
 * look at PDEs marked with ENTRY_TABLE_UNMAP to check if empty.
 */
void free_marked_page_tables(struct entry_table *pd) {
  entry_t *pde, *pte;
  int i, j;

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
        free_entry_table(pt);
      }

      *pde &= ~ENTRY_TABLE_UNMAP;
    }
  }
}

/**
 * @brief Unmap the set of pages.
 *
 * This function only unmaps the pages in the x86 virtual memory
 * data structures. It does not decrease the reference counter to the
 * unmapped pages or "free" them in any way. That is up to the caller.
 *
 * @param pd The page directory
 * @param addr The start address of the region to map
 * @param size The size of the region to map
 * @param ppages The physical addresses of the pages that were unmapped
 */
void __unmap_pages(struct entry_table *pd, size_t addr, size_t size, size_t *ppages) {
  size_t vpage;
  entry_t *pde, *pte;
  int num_pages = size / PAGE_SIZE;
  int i;

  for (i = 0; i < num_pages; i++) {
    vpage = addr + (i * X86_PAGE_SIZE);

    pde = get_pde(pd, vpage);

    /*
     * Mark the page _directory_ entry so we know that we unmapped a page
     * in this page table and we can free it later (if it's empty). Don't
     * mark the page directory entry if it global though because it is
     * being shared with other processes.
     */
    if (!entry_is_global(pde)) {
      *pde = *pde | ENTRY_TABLE_UNMAP;
    }
    else {
      /*
       * Actually...
       *  If the entry is global, there is probably a bug. Eventually it
       *  might be ok to unmap a global entry, but for now just panic.
       */
      panic("Trying to unmap global page: 0x%08x (virtual)", vpage);
    }

    ASSERT(entry_is_present(pde));

    /*
     * Mark the page _table_ entry as not present, effectively unmapping
     * the page.
     */
    pte = get_pte((struct entry_table *) entry_get_addr(pde), vpage);

    ASSERT(entry_is_present(pte));
    entry_set_absent(pte);

    ppages[i] = entry_get_addr(pte);
  }

  free_marked_page_tables(pd);
}

void unmap_pages(void *pd, size_t addr, size_t size, size_t *ppages) {
  TRACE("pd=%p, addr=0x%x, size=0x%x, ppages=%p", pd, addr, size, ppages);

  ASSERT(is_page_aligned(addr));
  ASSERT(is_page_aligned(size));

  return __unmap_pages((struct entry_table *) pd, addr, size, ppages);
}

/**
 * @brief Maps virt to phys with the given flags.
 *
 * @return
 *    0 on success, non-0 if the mapping could not be completed.
 *
 *    ENOMEM if mapping this page requires allocating a new page
 *      table data structure and the system has run out of available
 *      memory.
 */
static int map(struct entry_table *pd, size_t virt, size_t phys, int flags) {
  struct entry_table *pt;
  entry_t *pde, *pte;

  /*
   * Create a page directory if there is not one already present for the
   * 4MB chunk containing this page.
   */
  pde = get_pde(pd, virt);
  if (!entry_is_present(pde)) {
    pt = new_entry_table();
    if (!pt) {
      return ENOMEM;
    }

    entry_set_addr(pde, (size_t) pt);
    entry_set_present(pde);
    entry_set_flags(pde, VM_R | VM_W | VM_U);
  }
  else {
    pt = (struct entry_table *) entry_get_addr(pde);
  }
  ASSERT(entry_is_present(pde));

  /*
   * Write the physical address into the page table entry for the given
   * virtual address we are mapping.
   */
  pte = get_pte(pt, virt);
  entry_set_addr(pte, phys);
  entry_set_present(pte);
  entry_set_flags(pte, flags);

  {
    size_t _phys;
    ASSERT(__vtop(pd, virt, &_phys));
    ASSERT_EQUALS(phys, _phys);
  }

  return 0;
}

int __map_page(struct entry_table *pd, size_t virt, struct page *page, int flags) {
  unsigned i;

  /*
   * We don't assume the system-wide page size (PAGE_SIZE) is the same as the
   * page size on x86 (X86_PAGE_SIZE).
   */
  for (i = 0; i < PAGE_SIZE / X86_PAGE_SIZE; i++) {
    size_t v = virt + (i * X86_PAGE_SIZE);
    size_t p = page_address(page) + (i * X86_PAGE_SIZE);
    int ret;
   
    ret = map(pd, v, p, flags);

    //FIXME: rollback previous mappings if we fail
    // e.g.
    //    if (0 != ret) {
    //      __unmap(pd, virt, i * X86_PAGE_SIZE);
    //      return ret;
    //    }
    ASSERT_EQUALS(0, ret);
  }

  return 0;
}

/**
 * @brief Map a page of physical memory into the virtual address space.
 *
 * @param pd The address of the page directory
 * @param virt The virtual address to map
 * @param page The physical page to map.
 * @param flags
 *
 * @return 0 on success, non-0 on error
 */
int map_page(void *pd, size_t virt, struct page *page, int flags) {
  TRACE("pd=%p, virt=0x%x, page=0x%x, flags=%p", pd, virt, page_address(page), flags);

  ASSERT(is_page_aligned(virt));

  return __map_page((struct entry_table *) pd, virt, page, flags);
}


void x86_set_pagedir(struct entry_table *pd) {
  set_cr3((int32_t) pd);
}

struct entry_table *x86_get_pagedir(void) {
  return (struct entry_table *) get_cr3();
}

/**
 * @brief Flush the contents of the TLB, invalidating all cached virtual
 * address lookups.
 */
void tlb_flush(void) {
  set_cr3(get_cr3());
}

/**
 * @brief Invalidate a set of pages in the TLB. This should be called
 * after vm_map if you want to write to or read from the pages you just
 * mapped.
 */
void tlb_invalidate(size_t addr, size_t size) {
  (void) addr; (void) size;

  // TODO implement this with invlpg
  // e.g.
  //
  // addr = PAGE_ALIGN_DOWN( addr )
  // size = PAGE_ALIGN_UP( size )
  // for (p = addr; p < addr + size; p += page_size) x86_invlpg(p)
  //
  tlb_flush();
}
