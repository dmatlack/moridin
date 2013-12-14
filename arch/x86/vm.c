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

#include <mm/physmem.h>
#include <mm/vm.h>
#include <kernel/kmalloc.h>

#include <stddef.h>
#include <assert.h>
#include <debug.h>

#define IS_PAGE_ALIGNED(addr)\
  (FLOOR(X86_PAGE_SIZE, addr) == addr)

/*
 * The kernel page tables are used to map the kernel's virtual address
 * space. They are statically allocated because we need to initialize
 * virtual memory before we can use dynamic memory allocation. It is ok
 * that they are statically allocated because all processes will reuse
 * these page tables when mapping the kernel into their address space.
 */
struct entry_table kernel_pgtbls[NUM_KERNEL_PGTBLS]
  __attribute__((aligned(X86_PAGE_SIZE)));

struct vm_machine_interface x86_vm_machine_interface = {
  .bootstrap   = x86_vm_bootstrap,
  .init        = NULL,
  .init_object = x86_vm_init_object,
  .map         = x86_vm_map,
  .unmap       = NULL,
};

int x86_vm_init_object(struct vm_machine_object **object) {
  struct entry_table *page_directory;
  int ret;

  page_directory = entry_table_alloc();
  if (NULL == page_directory) {
    return ALLOC_FAILED;
  }

  ret = entry_table_init(page_directory);
  if (0 != ret) {
    entry_table_free(page_directory); 
    return ret;
  }

  *object = (struct vm_machine_object *) page_directory;
  return 0;
}

int x86_vm_map(struct vm_machine_object *object, size_t *vpages, 
               size_t *ppages, int num_pages, vm_flags_t flags) {
  return x86_map_pages((struct entry_table *) object, vpages, ppages,
                       num_pages, flags);
}

/**
 * @brief Initialize a page table or page directory (struct entry_table).
 *
 * This sets all entries in the table to 0. Most importantly, it marks all
 * entries as *not* present.
 */
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
 * @brief Allocate a new entry_table, with the proper memory address alignment.
 *
 * @return NULL if the allocation fails.
 */
struct entry_table *entry_table_alloc(void) {
  return kmemalign(X86_PAGE_SIZE, sizeof(struct entry_table));
}

void entry_table_free(struct entry_table *ptr) {
  kfree(ptr, sizeof(struct entry_table));
}

/**
 * @brief Set the entry flags to match the provided vm_flags_t.
 */
void entry_set_flags(entry_t *entry, vm_flags_t flags) {
  VM_IS_READWRITE(flags) ? 
      entry_set_readwrite(entry) :
      entry_set_readonly(entry);

  VM_IS_SUPERVISOR(flags) ?
      entry_set_supervisor(entry) :
      entry_set_user(entry);
}

/**
 * @brief Maps the virtual page, <vpage>, to the physical page, <ppage>.
 *
 * @return 0 on success, non-0 if the mapping could not be completed.
 *  Failure Scenarios:
 *    - Mapping this page requires allocating a new page table data
 *     structure and the system has run out of available memory.
 */
int x86_map_page(struct entry_table *pd, size_t vpage, size_t ppage,
                 vm_flags_t flags) {
  struct entry_table *pt;
  size_t vtop;
  entry_t *pde, *pte;

  ASSERT(IS_PAGE_ALIGNED(vpage));
  ASSERT(IS_PAGE_ALIGNED(ppage));

  pde = get_pde(pd, vpage);

  if (!entry_is_present(pde)) {
    pt = entry_table_alloc();

    if (NULL == pt) {
      return ALLOC_FAILED;
    }
    else {
      entry_table_init(pt);

      entry_set_addr(pde, (size_t) pt);
      entry_set_present(pde);
      entry_set_flags(pde, flags);
    }
  }

  ASSERT(entry_is_present(pde));

  pte = get_pte(pt, vpage);

  entry_set_addr(pte, ppage);
  entry_set_present(pte);
  entry_set_flags(pte, flags);

  ASSERT(x86_vtop(pd, vpage, &vtop) && ppage == vtop);

  return 0;
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
  int j;

  for (i = 0; i < num_pages; i++) {
    int ret = x86_map_page(pd, vpages[i], ppages[i], flags);

    /*
     * rollback all previous mappings 
     */
    if (0 != ret) {
      for (j = 0; j < i; j++) {
        //TODO x86_unmap_page
      }
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
