/**
 * @file x86/vm.h
 *
 * @brief Virtual Memory on an Intel x86 chip.
 *
 * Read more in Intel IA-32 System Architecture (Section 3.6)
 *
 * There are 4 data structures used in virtual memory:
 *
 * 1. Page Directory: An array of 32-bit "page directory entries" (PDEs),
 *      contained in a 4KB page.
 * 2. Page Table: An array of 32-bit "page table entries" (PTEs), contained
 *      in a 4KB page. Page Tables are not used for 2MB or 4MB pages.
 * 3. Pages: A 4KB, 2MB, or 4MB flat address space.
 * 4. Page-Directory-Pointer Table: An array of 4 64-bit entries, each of 
 *      which points to a page directory. This is only used when the physical
 *      address space extension (36-bit pointers) is used.
 *
 * @author David Matlack
 */
#ifndef __X86_VM_H__
#define __X86_VM_H__

#include <kernel/config.h>
#include <mm/vm.h>
#include <arch/x86/page.h>
#include <stdint.h>
#include <types.h>
#include <stddef.h>
#include <assert.h>

typedef int32_t entry_t;

/*
 * Page Directory & Page Table Entries for 4 KB pages
 * ----------------------------------------------------------------------------
 *
 * Page Directory Entries (PDEs) and Page Table Entries (PTEs) share a common
 * structure. They are both 32 bits, with the highest 20 bits indicating the
 * physical address of a page on the system. They also share the following
 * bit flags:
 */

/*  
 * Present (P) flag, bit 0
 *   Indicates whether the page table entry or physical page indicated by
 *   the upper 20 bits is loaded in memory. If this flag is set to 0 and
 *   an attempt is made to access the page, then a page fault (#PF) occurs.
 */
#define ENTRY_PRESENT 0
static inline void entry_set_present(entry_t *entry) { 
  set_bit(entry, ENTRY_PRESENT, 1);
}
static inline void entry_set_absent(entry_t *entry) {
  set_bit(entry, ENTRY_PRESENT, 0);
}
static inline bool entry_is_present(entry_t *entry) {
  return get_bit(*entry, ENTRY_PRESENT);
}

/*
 * Read/Write (RW) flag, bit 1
 *   When this flag is clear the page is read-only, when the flag is set
 *   the page can be read and written into.
 */
#define ENTRY_READWRITE 1
static inline void entry_set_readwrite(entry_t *entry) {
  set_bit(entry, ENTRY_READWRITE, 1);
}
static inline void entry_set_readonly(entry_t *entry) {
  set_bit(entry, ENTRY_READWRITE, 0);
}
static inline int entry_is_readwrite(entry_t *entry) {
  return get_bit(*entry, ENTRY_READWRITE);
}

/*
 * User/Supervisor (US) flag, bit 2
 *   Specifies the priveledge level of a page (PTE) or a group of pages
 *   (PDE). When the flag is clear, the page(s) are assigned the supervisor
 *   priveledge level. When the flag is set, the page(s) are assigned the
 *   user priveledge level.
 */
#define ENTRY_SUPERVISOR 2
static inline void entry_set_supervisor(entry_t *entry) {
  set_bit(entry, ENTRY_SUPERVISOR, 1);
}
static inline void entry_set_user(entry_t *entry) {
  set_bit(entry, ENTRY_SUPERVISOR, 0);
}
static inline int entry_is_supervisor(entry_t *entry) {
  return get_bit(*entry, ENTRY_SUPERVISOR);
}

/*
 * Page-level write-through (PWT) flag, bit 3
 *   Controls the write-through and write-back caching policy of individual
 *   pages or page tables. When the flag is set, write-through is enabled.
 *   When the flag is clear, write-back is enabled. The processor ignores
 *   this flag if the cache disable (CD) flag of CR0 is set.
 */
#define ENTRY_WRITETHROUGH 3

/*
 * Page-level cache disable (PCD) flag, bit 4
 *   Controls the caching of individual pages or page tables. When the
 *   flag is set, caching is disabled. This flag can be used to prevent
 *   caching of pages that contain memory mapped I/O ports.
 */
#define ENTRY_CACHEDISABLED 4
static inline void entry_disable_cache(entry_t *entry) {
  set_bit(entry, ENTRY_CACHEDISABLED, 1);
}
static inline void entry_enable_cache(entry_t *entry) {
  set_bit(entry, ENTRY_CACHEDISABLED, 0);
}

/*
 * Accessed (A) flag, bit 5
 *   Indicates whether the page or page table has been accessed (read from
 *   or written to). Once this flag has been set by the processor, it is
 *   up to the software to clear it.
 */
#define ENTRY_ACCESSED 5

/*
 * Dirty (D) flag, bit 6 (Page Table only)
 *   Indicates that the page has been written to. Like the Accessed (A) flag,
 *   it is up to the software to clear this flag after it has been set by
 *   the processor.
 */
#define ENTRY_DIRTY 6
static inline void entry_clear_dirty(entry_t *entry) {
  set_bit(entry, ENTRY_DIRTY, 0);
}
static inline int entry_is_dirty(entry_t *entry) {
  return get_bit(*entry, ENTRY_DIRTY);
}

/*
 * Page size (PS) flag, bit 7 (Page Directory Only)
 *   Determines the page size. When the flag is clear, pages are 4KB and the
 *   PDE points to a Page Table.
 */
#define ENTRY_PAGESIZE4MB 7

/* 
 * Page attribute index table (PAT) flag, bit 7 (Page Table only)
 *   This flag is used along with the PCD and PWT flags to select an entry in 
 *   the Page Attribute Table (PAT).
 *
 *   ONLY FOR > PENTIUM III PROCESSORS
 */
#define ENTRY_ATTRIBUTEINDEXTABLE 7

/*
 * Global (G) flag, bit 8
 *   Indicates a global page when set. When a page is marked global and the PGE
 *   flag of CR4 is set, the PTE or PDE is not invalidated in the TLB on a
 *   task switch or write to CR3.
 *
 *   ONLY FOR > PENTIUM PRO PROCESSORS
 */
#define ENTRY_GLOBAL 8

/*
 * Available, bits 9, 10, 11
 *   These bits are available for use by the system programmer.
 */
#define ENTRY_AVAIL 9
#define   ENTRY_TABLE_UNMAP (1 << 9) // used to mark page directory entries
#define ENTRY_AVAIL_MASK MASK(3)

/*
 * Page Table Base Address (PT) or Physical Page Address (PP)
 *   bits 12-31
 */
#define ENTRY_ADDR_MASK (~MASK(11))
static inline void entry_set_addr(entry_t *entry_ptr, size_t addr) {
  ASSERT_EQUALS(FLOOR(X86_PAGE_SIZE, (size_t) (addr)), ((size_t)(addr)));
  *(entry_ptr) &= ~ENTRY_ADDR_MASK;
  *(entry_ptr) |= addr;
}
static inline size_t entry_get_addr(entry_t *entry) {
  return (size_t) ((*entry) & ENTRY_ADDR_MASK);
}

/*
 * Page Directories and Page Tables are really two of the same
 * thing: an array of entries. Thus we will represent each with
 * the same data structure.
 */
#define ENTRY_TABLE_SIZE ((X86_PAGE_SIZE) / sizeof(entry_t))
struct entry_table {
  entry_t entries[ENTRY_TABLE_SIZE];
};

int entry_table_init(struct entry_table *tbl);
struct entry_table *entry_table_alloc(void);
void entry_table_free(struct entry_table *ptr);

/*
 * Linear Address translation for 4 KB pages
 * ----------------------------------------------------------------------------
 *        
 *                 +-----------+-----------+-----------+
 * Linear Address: | Directory | Table     | Offset    |
 *                 +-----------+-----------+-----------+
 *                  31       22 21       12 11        0
 *
 *      22-31 (Directory)  Offset into the Page Directory, giving you the
 *                         address of the Page Table.
 *
 *      12-21 (Table)      Offset into the Page Table, giving you the physical
 *                         address of the page.
 *
 *      0-11  (Offset)     Offset into the physical page.
 *
 *      (Note that cr3 is the starting point of this translation since it
 *       holds the address of the Page Directory).
 *
 */
#define PD_OFFSET(la)  (((la) >> 22) & MASK(10))
#define PT_OFFSET(la)  (((la) >> 12) & MASK(10))
#define PHYS_OFFSET(la) ((la) & MASK(12))

static inline entry_t* get_pde(struct entry_table *pd, size_t vaddr) {
  return &(pd->entries[PD_OFFSET(vaddr)]);
}

static inline entry_t* get_pte(struct entry_table *pt, size_t vaddr) {
  return &(pt->entries[PT_OFFSET(vaddr)]);
}

int x86_init_page_dir(struct entry_table **pd);

bool x86_vtop(struct entry_table *pd,  size_t vaddr, size_t *paddrp);

int x86_map_pages(struct entry_table *pd, size_t addr, size_t size,
                  size_t *ppages, vm_flags_t flags);

void x86_unmap_pages(struct entry_table *pd, size_t addr, size_t size,
                     size_t *ppages);

struct entry_table *x86_get_pagedir(void);
void x86_set_pagedir(struct entry_table *pd);

#endif /* !__X86_VM_H__ */
