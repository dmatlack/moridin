/**
 * @file x86/bootstrap_vm.c
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

#define NUM_PAGES_IN_ZONE_KERNEL (CONFIG_MAX_KERNEL_MEM / X86_PAGE_SIZE)

/*
 * We must +1 the number of tables in case the kernel's virtual memory
 * zone is layed out such that it starts near the end of one page
 * table and ends at the beginning of the last page table.
 */
#define NUM_BOOTSTRAP_KERNEL_PGTBLS\
  ((NUM_PAGES_IN_ZONE_KERNEL / ENTRY_TABLE_SIZE) + 1)

/*
 * Low memory includes the BIOS region, the kernel image. 2 MB should be
 * enough... FIXME
 */
#define BOOTSTRAP_LOWMEM_SIZE MB(2)
#define NUM_BOOTSTRAP_LOWMEM_PGTBLS\
  ((BOOTSTRAP_LOWMEM_SIZE / X86_PAGE_SIZE / ENTRY_TABLE_SIZE + 1))

static struct entry_table 
  bootstrap_kernel_pgtbls[NUM_BOOTSTRAP_KERNEL_PGTBLS]
  __attribute__((aligned(X86_PAGE_SIZE)));

static struct entry_table
  bootstrap_lowmem_pgtbls[NUM_BOOTSTRAP_LOWMEM_PGTBLS]
  __attribute__((aligned(X86_PAGE_SIZE)));

static struct entry_table 
  bootstrap_pgdir[1] 
  __attribute__((aligned(X86_PAGE_SIZE)));

/**
 * @brief Bootstrap a region of memory to be mapped by mapping page directory
 * entries to page tables so that the real VM mapping code does not try to 
 * "malloc" a new page table.
 *
 * @param pd the page directory 
 * @param pts an array of page tables to use for the bootstrapping
 *
 * @param vstart the start address of the virtual region to bootstrap
 * @param vsize the size, in bytes, of the virtual region to bootstrap
 *
 * @param pstart the start address of the physical memory we will use
 * to back this virtual memory
 */
static void x86_bootstrap_region(struct entry_table *pd,
    struct entry_table *pts, size_t vstart, size_t vsize, size_t pstart) {

  unsigned int num_pgtbls;
  unsigned int i;
  size_t vpage;
  size_t ppage;
  size_t vend;

  TRACE("pd=%p, pts=%p, vstart=0x%08x, vsize=0x%08x, pstart=0x%08x",
        pd, pts, vstart, vsize, pstart);

  /* 
   * vpage is the virtual address of the first page of each page table
   */
  vpage = FLOOR(X86_PAGE_SIZE * ENTRY_TABLE_SIZE, vstart);

  vend = CEIL(X86_PAGE_SIZE * ENTRY_TABLE_SIZE, vstart + vsize);
  num_pgtbls = (vend - vpage) / X86_PAGE_SIZE / ENTRY_TABLE_SIZE;

  for (i = 0; i < num_pgtbls; i++) {
    entry_t *pde = get_pagedir_entry(pd, vpage);
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
    
    vpage += X86_PAGE_SIZE * ENTRY_TABLE_SIZE;
  }

  for (i = 0; i < vsize / X86_PAGE_SIZE; i++) {
    struct entry_table *pt;
    entry_t *pde, *pte;

    vpage = vstart + i*X86_PAGE_SIZE;
    ppage = pstart + i*X86_PAGE_SIZE;

    pde = get_pagedir_entry(pd, vpage);

    assert(entry_is_present(pde));

    pt = (struct entry_table *) entry_get_addr(pde);
    pte = get_pagetbl_entry(pt, vpage);

    entry_set_present(pte);
    entry_set_supervisor(pte);
    entry_set_readwrite(pte);
    entry_set_addr(pte, ppage);

    assert(ppage == vtop(pd, vpage));
  }
}
/**
 * @brief Bootrap into virtual memory using some static page dir/tables.
 *
 * This function is expected to initialize the virtual memory data structures
 * to map VM_ZONE_KERNEL to PMEM_ZONE_KERNEL, where the size of each is
 * maximally CONFIG_MAX_KERNEL_MEM.
 */
int x86_vm_bootstrap(size_t kernel_page_size) {
  TRACE("kernel_page_size=0x%08x", kernel_page_size);

  assert(sizeof(entry_t) == 4);
  assert(X86_PAGE_SIZE == KB(4));
  assert(X86_PAGE_SIZE == kernel_page_size);
  assert(VM_ZONE_KERNEL->size == PMEM_ZONE_KERNEL->size);
  assert(VM_ZONE_KERNEL->size < CONFIG_MAX_KERNEL_MEM);
  assert(FLOOR(X86_PAGE_SIZE, (size_t) bootstrap_pgdir) == 
         (size_t) bootstrap_pgdir);

  DEBUG("bootstrap_pgdir=0x%08x", bootstrap_pgdir);
  DEBUG("bootstrap_kernel_pgtbls=0x%08x", bootstrap_kernel_pgtbls);
  DEBUG("bootstrap_lowmem_pgtbls=0x%08x", bootstrap_lowmem_pgtbls);

  entry_table_init(bootstrap_pgdir);
  
  /*
   * direct map low memory so that our kernel can still run after we enable
   * paging
   */
  x86_bootstrap_region(bootstrap_pgdir, 
                       bootstrap_lowmem_pgtbls,
                       0, 
                       BOOTSTRAP_LOWMEM_SIZE,
                       0);

  /*
   * map the kernel's virtual address space to a zone in physical memory
   */
  x86_bootstrap_region(bootstrap_pgdir, 
                       bootstrap_kernel_pgtbls,
                       VM_ZONE_KERNEL->address, 
                       VM_ZONE_KERNEL->size,
                       PMEM_ZONE_KERNEL->address);


  DEBUG("Setting pagedir (cr3)...");
  DEBUG("cr3 (before) = 0x%08x", get_cr3());
  x86_set_pagedir((int) bootstrap_pgdir);
  DEBUG("cr3 (after)  = 0x%08x", get_cr3());
  
  DEBUG("Enable global pages (setting bit %d of cr4)...", CR4_PGE);
  DEBUG("cr4 (before) = 0x%08x", get_cr4());
  x86_enable_global_pages();
  DEBUG("cr4 (after)  = 0x%08x", get_cr4());

  DEBUG("Enable write protect (setting bit %d of cr0)...", CR0_WP);
  DEBUG("cr0 (before) = 0x%08x", get_cr0());
  x86_enable_write_protect();
  DEBUG("cr0 (after)  = 0x%08x", get_cr0());

  /*
   * officially turn the virtual memory system "on"
   */
  DEBUG("About to enable paging... (setting bit %d of cr0)", CR0_PG);
  DEBUG("cr0 (before) = 0x%08x", get_cr0());
  x86_enable_paging();

  return 0;
}

