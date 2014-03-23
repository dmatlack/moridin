/**
 * @file x86/vm.c
 *
 * @brief x86 Virtual Memory Implementation
 *
 */
#include <arch/vm.h>
#include <arch/page.h>
#include <arch/reg.h>
#include <arch/cpu.h>

#include <mm/kmalloc.h>

#include <mm/memory.h>
#include <mm/vm.h>
#include <mm/pages.h>

#include <stddef.h>
#include <assert.h>
#include <kernel/debug.h>
#include <errno.h>

#define DEFAULT_PDE_FLAGS (VM_P | VM_R | VM_W | VM_U)

static inline bool is_page_aligned(unsigned long addr) {
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
 * @brief This function is responsible for making to_pd map the same address
 * space as from_pd.
 *
 * If this function succeeds, the following will be true:
 *    1. Any virtual address that has a present mapping in from_pd will also
 *       have a mapping in to_pd.
 *    2. All kernel virtual addresses in from_pd will be mapped into to_pd by
 *       only copying the page directory entries.
 *    3. All user virtual addresses in from_pd will be mapped into to_pd by 
 *       allocating a new page table for to_pd.
 *    4. For each mapping (from_virt:from_phys) and (to_virt:to_phys), if
 *       from_virt == to_virt then from_phys == to_phys.
 *    5. All userspace mappings in to_pd and from_pd will be read-only.
 *
 * @return 0 on success, non-0 on error
 */
int fork_address_space(struct entry_table *to_pd, struct entry_table *from_pd) {
  unsigned i;

  TRACE("to_pd=0x%08x, from_pd=0x%08x", to_pd, from_pd);

  for (i = 0; i < ENTRY_TABLE_SIZE; i++) {
    unsigned long virt = i * (PAGE_SIZE * ENTRY_TABLE_SIZE);
    entry_t *from_pde = from_pd->entries + i;
    entry_t *to_pde = to_pd->entries + i;

    /*
     * All processes share the same page tables to map the kernel. So
     * the only thing we need to copy for a kernel mapping is to copy
     * the page directory entries.
     */
    if (kernel_address(virt)) {
      *to_pde = *from_pde;
    }
    /*
     * If it's a user address, and the page directory entry is present, we
     * assume that it's pointing to a page table and we make a copy of that
     * page table instead.
     *
     * WARNING: using 4MB pages (PSE) will break this code.
     */
    else if (entry_is_present(from_pde)) {
      struct entry_table *to_pt;
      struct entry_table *from_pt;
      unsigned j;

      from_pt = (struct entry_table *) entry_get_addr(from_pde);

      to_pt = new_entry_table();
      if (!to_pt) {
        panic("TODO: unmap all pages.");
      }

      /*
       * Copy the page directory entry.
       */
      *to_pde = *from_pde;

      /*
       * But use the address of the newly created page table.
       */
      entry_set_addr(to_pde, (unsigned long) to_pt);

      for (j = 0; j < ENTRY_TABLE_SIZE; j++) {
        entry_t *from_pte = from_pt->entries + j;
        entry_t *to_pte = to_pt->entries + j;

        /*
         * Copy the page table entry.
         */
        *to_pte = *from_pte;

        /*
         * If the entry is mapped to a physical page, increase the reference
         * count on that page by 1 as it is being used in the new address
         * space.
         */
        if (entry_is_present(from_pte)) {
          unsigned long phys = (unsigned long) entry_get_addr(from_pte);

          page_get(page_struct(phys));

          /*
           * Mark the page readonly in both page tables so both processes will 
           * page fault on a write and we can copy the page.
           */
          entry_set_readonly(from_pte);
          entry_set_readonly(to_pte);
        }
      }
    }
  }

  return 0;
}

/**
 * @brief This function looks at the flags (see mm/vm.h) and sets the
 * correct bits in the page (table|directory) entry.
 */
void entry_set_flags(entry_t *entry, int flags) {
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

  if (flags & VM_G) {
    entry_set_global(entry);
  }

  if (flags & VM_P) {
    entry_set_present(entry);
  }
  else {
    entry_set_absent(entry);
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
bool __vtop(struct entry_table *pd, unsigned long v, unsigned long *pp) {
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
    if (entry_is_present(pde) && (*pde & ENTRY_TABLE_UNMAP)) {
      struct entry_table *pt = (struct entry_table *) entry_get_addr(pde);
      bool page_table_empty = true;

      for (j = 0; j < (int) ENTRY_TABLE_SIZE; j++) {
        pte = pt->entries + j;
        if (entry_is_present(pte)) {
          page_table_empty = false;
          break;
        }
      }

      /*
       * If the page table was empty, mark the page directory entry as
       * absent and free the page table.
       */
      if (page_table_empty) {
        entry_set_absent(pde);
        free_entry_table(pt);
      }

      *pde &= ~ENTRY_TABLE_UNMAP;
    }
  }
}

struct page *unmap_page(void *pd, unsigned long virt) {
  unsigned long vpage;
  entry_t *pde, *pte;
  int i;
  struct page *page = NULL;

  TRACE("pd=%p, virt=0x%x", pd, virt);

  for (i = 0; i < PAGE_SIZE / X86_PAGE_SIZE; i++) {
    vpage = virt + (i * X86_PAGE_SIZE);

    pde = get_pde(pd, vpage);

    if (entry_is_present(pde)) {
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

      pte = get_pte((struct entry_table *) entry_get_addr(pde), vpage);

      /*
       * This page wasn't mapped in the first place. Return NULL to indicate
       * no page was unmapped.
       */
      if (!entry_is_present(pte)) {
        return NULL;
      }

      /*
       * Mark the page _table_ entry as not present, effectively unmapping
       * the page.
       */
      entry_set_absent(pte);
      if (!page) page = page_struct(entry_get_addr(pte));
    }
  }

  free_marked_page_tables(pd);

  return page;
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
static int map(struct entry_table *pd, unsigned long virt, unsigned long phys, int flags) {
  struct entry_table *pt;
  entry_t *pde, *pte;

  pde = get_pde(pd, virt);

  /*
   * Create a page table if there is not one already present for the
   * 4MB chunk containing this page.
   */
  if (!entry_is_present(pde)) {
    pt = new_entry_table();
    if (!pt) {
      return ENOMEM;
    }

    entry_set_addr(pde, (unsigned long) pt);
    entry_set_flags(pde, DEFAULT_PDE_FLAGS);
  }
  
  pt = (struct entry_table *) entry_get_addr(pde);
  ASSERT(entry_is_present(pde));

  /*
   * Write the physical address into the page table entry for the given
   * virtual address we are mapping.
   */
  pte = get_pte(pt, virt);
  entry_set_addr(pte, phys);
  entry_set_flags(pte, flags);

  {
    unsigned long _phys;
    ASSERT(__vtop(pd, virt, &_phys));
    ASSERT_EQUALS(phys, _phys);
  }

  return 0;
}

int __map_page(struct entry_table *pd, unsigned long virt, struct page *page, int flags) {
  unsigned i;

  /*
   * We don't assume the system-wide page size (PAGE_SIZE) is the same as the
   * page size on x86 (X86_PAGE_SIZE).
   */
  for (i = 0; i < PAGE_SIZE / X86_PAGE_SIZE; i++) {
    unsigned long v = virt + (i * X86_PAGE_SIZE);
    unsigned long p = page_address(page) + (i * X86_PAGE_SIZE);
    int ret;
   
    ret = map(pd, v, p, flags);
    
    if (ret) {
      unmap_page(pd, virt);
      return ret;
    }
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
int map_page(void *pd, unsigned long virt, struct page *page, int flags) {
  TRACE("pd=%p, virt=0x%x, page=0x%x, flags=%p", pd, virt, page_address(page), flags);

  ASSERT(is_page_aligned(virt));

  return __map_page((struct entry_table *) pd, virt, page, flags);
}

/**
 * @brief This is the main page fault handling routine for arch/x86. It's job
 * is to parse the architecture generated exception and pass it up to the kernel
 * virtual memory manager to be handled.
 */
void page_fault(int vector, int error, struct registers *regs) {
  int flags = 0;
  int ret;

  TRACE("vector=%d, error=%d, regs=%p", vector, error, regs);

  if (error & 1) flags |= PF_PRESENT;
  if (error & 2) flags |= PF_WRITE;
  else           flags |= PF_READ;
  if (error & 4) flags |= PF_USER;
  else           flags |= PF_SUPERVISOR;

  ret = vm_page_fault(regs->cr2, flags);

  if (ret) {
    //TODO: kill the process
    exn_panic(vector, error, regs);
  }
}
