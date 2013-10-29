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


/*
 * The bootstrap page directory and page table(s) are needed because we need
 * some virtual memory to exist before we can use kmalloc. This means we
 * need some static tables that can be used to run the virtual memory system.
 */
struct x86_pgdir bootstrap_pgdir;
/*
 * We need enough page tables to map the entire kernel memory zone. We know
 * the maximum ammount of kernel memory so we'll just make enough page tables
 * for that and we'll be good.
 */
#define NUM_PAGES_IN_ZONE_KERNEL CONFIG_MAX_KERNEL_MEM / X86_PAGE_SIZE
#define NUM_BOOTSTRAP_PGTBLS NUM_PAGES_IN_ZONE_KERNEL / X86_PT_SIZE
struct x86_pgtbl bootstrap_pgtbls[NUM_BOOTSTRAP_PGTBLS];

/**
 * @brief Bootrap into virtual memory using some static page dir/tables.
 *
 * This function does some basic x86 virtual memory checks and initializations
 * but also initializes the static bootstrap page directory and page tables, 
 * mapping all addresses in high memory to the kernel's pmem zone.
 */
int x86_vm_bootstrap(size_t kernel_page_size) {
  int e;
  int i;

  assert(sizeof(int) == 4);
  assert(X86_PAGE_SIZE == KB(4));
  assert(X86_PAGE_SIZE == kernel_page_size);

  /*
   * The default setting of flags for page directory/table entries.
   */
  e = 0;
  SET_BIT(e, PDE_PRESENT,   0); // the entry is not present
  SET_BIT(e, PDE_PCD,       1); // disable caching
  SET_BIT(e, PDE_PS,        0); // pages are 4KB
  __initial_entry = e;

  /*
   * Bootstrap in Virtual Memory:
   * 
   * 1. Initialize all the virtual memory data structures.
   */
  x86_pgdir_init(&bootstrap_pgdir);
  for (i = 0; i < NUM_BOOTSTRAP_PGTBLS; i++) {
    x86_pgtbl_init(&bootstrap_pgtbls[i]);
  }

  /*
   * 2. Reserve the kernel's memory zone since we will be taking it.
   */
  pmem_alloc_zone(PMEM_ZONE_KERNEL);

  /*
   * 3. Map the VM_ZONE_KERNEL to the physical pages in PMEM_ZONE_KERNEL.
   */


  return 0;
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

struct x86_pgdir *x86_pgdir_alloc(void) {
  return kmemalign(X86_PAGE_SIZE, sizeof(struct x86_pgdir));
}
void x86_pgdir_free(struct x86_pgdir *pd) {
  kfree(pd, sizeof(struct x86_pgdir));
}

/**
 * @brief Initialize a page table by setting all default flags and marking
 * each entry as NOT PRESENT.
 */
int x86_pgtbl_init(struct x86_pgtbl *pt) {
  int i;
  for (i = 0; i < X86_PT_SIZE; i++) {
    pt->entries[i] = __initial_entry;
  }
  return 0;
}

struct x86_pgtbl *x86_pgtbl_alloc(void) {
  return kmemalign(X86_PAGE_SIZE, sizeof(struct x86_pgtbl));
}
void x86_pgtbl_free(struct x86_pgtbl *pt) {
  kfree(pt, sizeof(struct x86_pgtbl));
}

