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
#include <mm/vm.h>
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

static inline void __entry_set_addr(x86_entry_t *entry, size_t addr) {
  assert(PAGE_ALIGN_DOWN(addr) == addr);
  *entry = (*entry) & ~(PDE_PT_MASK << PDE_PT);
  *entry = (*entry) & addr;
}

static inline void __entry_set_present(x86_entry_t *entry) {
  set_bit(entry, PDE_PRESENT, 1); 
}

/**
 * @brief Bootrap into virtual memory using some static page dir/tables.
 *
 * This function is expected to initialize the virtual memory data structures
 * to map VM_ZONE_KERNEL to PMEM_ZONE_KERNEL, where the size of each is
 * maximally CONFIG_MAX_KERNEL_MEM.
 */
int x86_vm_bootstrap(size_t kernel_page_size) {
  int e;
  int i;
  size_t vpage;

  assert(sizeof(int) == 4);
  assert(X86_PAGE_SIZE == KB(4));
  assert(X86_PAGE_SIZE == kernel_page_size);

  assert(VM_ZONE_KERNEL->size = PMEM_ZONE_KERNEL->size);
  assert(VM_ZONE_KERNEL->size < CONFIG_MAX_KERNEL_MEM);

  /*
   * The default setting of flags for page directory/table entries.
   */
  e = 0;
  set_bit(&e, PDE_PRESENT,   0); // the entry is not present
  set_bit(&e, PDE_PCD,       1); // disable caching
  set_bit(&e, PDE_PS,        0); // pages are 4KB
  __initial_entry = e;

  /*
   * Bootstrap in Virtual Memory:
   * 
   * 1. Initialize the page directory
   */
  x86_pgdir_init(&bootstrap_pgdir);

  /*
   * 2. Map the Page Directory to the Page Tables so that VM_ZONE_KERNEL is
   *    covered.
   */
  i = 0;
  for (vpage = FLOOR(X86_PAGE_SIZE * X86_PT_SIZE, VM_ZONE_KERNEL->address);
       vpage < VM_ZONE_KERNEL->address + VM_ZONE_KERNEL->size;
       vpage += X86_PAGE_SIZE * X86_PT_SIZE) {

    /* 
     * vpage is the virtual address of the first page of mapped by the page 
     * table
     */
    size_t pd_index = PD_INDEX(vpage);
    x86_entry_t *pde = bootstrap_pgdir.entries + pd_index;
    struct x86_pgtbl *pgtbl = bootstrap_pgtbls + i;

    /*
     * set the page directory entry to use one of the bootrap page tables
     */
    __entry_set_addr(pde, (size_t) pgtbl);
    __entry_set_present(pde);
    x86_pgtbl_init(pgtbl);

    i++;
  }

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
