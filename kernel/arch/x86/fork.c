/**
 * @file arch/x86/fork.c
 *
 * @brief x86-specific functions for performing a process fork.
 */
#include <arch/vm.h>
#include <kernel/proc.h>
#include <stddef.h>

extern void __fork_context(void **save_addr, void *restore_addr);

void fork_context(struct thread *new_thread) {
  TRACE("new_thread=%p", new_thread);
  __fork_context(&new_thread->context, CURRENT_THREAD->context);
}

/**
 * @brief This function is called half-way through a fake context switch.
 * 
 * @param addr This is the address of the thread struct's "context" pointer. This
 * address is needed so that copy_context can figure out the address of the
 * thread struct itself.
 */
void copy_context(void **addr) {
  struct thread *new_thread = container_of(addr, struct thread, context);

  TRACE("new_thread=%p", new_thread);

  /*
   * Copy the entire kernel stack from the current thread to the new thread.
   */
  memcpy((void *) _KSTACK_START(new_thread), (void *) _KSTACK_START(CURRENT_THREAD), KSTACK_SIZE);

  /*
   * Set the new thread's context pointer to point to it's own stack instead of
   * its parent's stack.
   */
  new_thread->context = (void *) (((size_t) (*addr)) - (size_t) CURRENT_THREAD + (size_t) new_thread);
}

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

      from_pt = entry_pt(from_pde);

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
      entry_set_addr(to_pde, __phys(to_pt));

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
          unsigned long phys = entry_phys(from_pte);

          /*
           * Increase the reference counter on the page since another page table
           * now points to it.
           */
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

