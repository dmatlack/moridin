/**
 * @file x86/bootstrap_vm.c
 *
 * @author David Matlack
 */
#include <arch/x86/vm.h>
#include <arch/x86/page.h>
#include <arch/x86/reg.h>
#include <arch/x86/cpu.h>

#include <mm/physmem.h>
#include <mm/vm.h>
#include <kernel/kmalloc.h>

#include <stddef.h>
#include <assert.h>
#include <debug.h>

static struct entry_table 
  bootstrap_pgdir[1] 
  __attribute__((aligned(X86_PAGE_SIZE)));

/**
 * @brief Map up to <psize> of the virtual region starting at <vstart> into
 * the physical region starting at <pstart>, marking the rest of the pages
 * in the virtual region as NOT PRESENT.
 *
 * Present pages will be marked read write, and with supervisor priveledges.
 *
 * @param pd the page directory 
 *
 * @param pts an array of page tables to use for the bootstrapping
 *
 * @param vstart the start address of the virtual region to bootstrap
 * @param vsize the size, in bytes, of the virtual region to bootstrap
 *
 * @param pstart the start address of the physical memory we will use
 * to back this virtual memory
 * @param psize the size of the physical region backing the virtual region
 */
static void x86_bootstrap_region(struct entry_table *pd,
    struct entry_table *pts, size_t vstart, size_t vsize, 
    size_t pstart, size_t psize) {

  unsigned int num_pgtbls;
  unsigned int i;
  size_t vpage;
  size_t ppage;
  size_t vend;

  TRACE("pd=%p, pts=%p, vstart=0x%08x, vsize=0x%08x, pstart=0x%08x, psize=0x%08x",
        pd, pts, vstart, vsize, pstart, psize);

  /* 
   * vpage is the virtual address of the first page of each page table
   */
  vpage = FLOOR(X86_PAGE_SIZE * ENTRY_TABLE_SIZE, vstart);
  DEBUG("    vpage=0x%08x", vpage);

  vend = CEIL(X86_PAGE_SIZE * ENTRY_TABLE_SIZE, vstart + vsize);
  DEBUG("    vend=0x%08x", vend);

  num_pgtbls = (vend - vpage) / X86_PAGE_SIZE / ENTRY_TABLE_SIZE;
  DEBUG("    num_pgtbls=%d", num_pgtbls);

  for (i = 0; i < num_pgtbls; i++) {
    entry_t *pde = get_pde(pd, vpage);
    size_t pgtbl_addr = (size_t) &pts[i];
    
    /*
     * initialize the actual page table
     */
    entry_table_init(pts + i);

    /*
     * map the address of the page table in this page directory entry
     */
    entry_set_addr(pde, pgtbl_addr);

    entry_set_present(pde);
    entry_set_readwrite(pde);

    ASSERT(entry_get_addr(pde) == pgtbl_addr);

    vpage += X86_PAGE_SIZE * ENTRY_TABLE_SIZE;
  }

  /*
   * map each virtual page address to its corresponding, direct mapped,
   * physical page
   */
  for (i = 0; i < psize / X86_PAGE_SIZE; i++) {
    struct entry_table *pt;
    entry_t *pde, *pte;

    vpage = vstart + i*X86_PAGE_SIZE;
    ppage = pstart + i*X86_PAGE_SIZE;

    pde = get_pde(pd, vpage);
    ASSERT(entry_is_present(pde));

    pt = (struct entry_table *) entry_get_addr(pde);
    pte = get_pte(pt, vpage);

    entry_set_present(pte);
    entry_set_supervisor(pte);
    entry_set_readwrite(pte);
    entry_set_addr(pte, ppage);

    ASSERT(ppage == entry_get_addr(pte));
    ASSERT(ppage == x86_vtop(pd, vpage));
  }

  DEBUG("    MAPPED: 0x%08x through 0x%08x", vstart, vstart + psize);
  DEBUG("    NOT MAPPED: 0x%08x thorugh 0x%08x", vstart + psize, vend);

}

/**
 * @brief Bootrap into virtual memory using some static page dir/tables.
 *
 * This function is expected to initialize the virtual memory data structures
 * to map VM_ZONE_KERNEL to PMEM_ZONE_KERNEL.
 */
int x86_vm_bootstrap(size_t kernel_page_size) {
  TRACE("kernel_page_size=0x%08x", kernel_page_size);

  ASSERT(sizeof(entry_t) == 4);
  ASSERT(X86_PAGE_SIZE == KB(4));
  ASSERT(X86_PAGE_SIZE == kernel_page_size);
  ASSERT(FLOOR(X86_PAGE_SIZE, (size_t) bootstrap_pgdir) == 
         (size_t) bootstrap_pgdir);

  entry_table_init(bootstrap_pgdir);

  x86_bootstrap_region(bootstrap_pgdir, 
                       kernel_pgtbls,
                       VM_ZONE_KERNEL->address, 
                       VM_ZONE_KERNEL->size,
                       PMEM_ZONE_KERNEL->address,
                       PMEM_ZONE_KERNEL->size);

  x86_set_pagedir((int) bootstrap_pgdir);
  x86_enable_global_pages();
  x86_enable_write_protect();

  /*
   * officially turn the virtual memory system "on"
   */
  x86_enable_paging();

  return 0;
}

