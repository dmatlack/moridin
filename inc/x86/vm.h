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

#include <x86/page.h>
#include <stdint.h>
#include <types.h>

#define X86_PD_SIZE (X86_PAGE_SIZE / sizeof(int32_t))
#define X86_PT_SIZE (X86_PAGE_SIZE / sizeof(int32_t))

typedef int32_t x86_vme_t;

struct x86_pg_dir {
  x86_vme_t entries[X86_PD_SIZE];
};

struct x86_pg_tbl {
  x86_vme_t entries[X86_PT_SIZE];
};

int x86_vm_init(size_t kernel_page_size);


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
#define PDE_OFFSET(la)  (((la) >> 22) & MASK(10))
#define PTE_OFFSET(la)  (((la) >> 12) & MASK(10))
#define PHYS_OFFSET(la) ((la) & MASK(12))

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
#define PDE_PRESENT   0
#define PTE_PRESENT   0

/*
 * Read/Write (RW) flag, bit 1
 *   When this flag is clear the page is read-only, when the flag is set
 *   the page can be read and written into.
 */
#define PDE_RW        1
#define PTE_RW        1

/*
 * User/Supervisor (US) flag, bit 2
 *   Specifies the priveledge level of a page (PTE) or a group of pages
 *   (PDE). When the flag is clear, the page(s) are assigned the supervisor
 *   priveledge level. When the flag is set, the page(s) are assigned the
 *   user priveledge level.
 */
#define PDE_US        2
#define PTE_US        2

/*
 * Page-level write-through (PWT) flag, bit 3
 *   Controls the write-through and write-back caching policy of individual
 *   pages or page tables. When the flag is set, write-through is enabled.
 *   When the flag is clear, write-back is enabled. The processor ignores
 *   this flag if the cache disable (CD) flag of CR0 is set.
 */
#define PTE_PTW       3
#define PDE_PTW       3

/*
 * Page-level cache disable (PCD) flag, bit 4
 *   Controls the caching of individual pages or page tables. When the
 *   flag is set, caching is disabled. This flag can be used to prevent
 *   caching of pages that contain memory mapped I/O ports.
 */
#define PDE_PCD       4
#define PTE_PCD       4

/*
 * Accessed (A) flag, bit 5
 *   Indicates whether the page or page table has been accessed (read from
 *   or written to). Once this flag has been set by the processor, it is
 *   up to the software to clear it.
 */
#define PDE_A         5
#define PTE_A         5

/*
 * Dirty (D) flag, bit 6 (PTE ONLY)
 *   Indicates that the page has been written to. Like the Accessed (A) flag,
 *   it is up to the software to clear this flag after it has been set by
 *   the processor.
 */
#define PDE_RESERVED  6
#define PTE_D         6

/*
 * Page size (PS) flag, bit 7 (PDE ONLY)
 *   Determines the page size. When the flag is clear, pages are 4KB and the
 *   PDE points to a Page Table.
 */
#define PDE_PS        7

/* 
 * Page attribute index table (PAT) flag, bit 7 (PTE ONLY)
 *   This flag is used along with the PCD and PWT flags to select an entry in 
 *   the Page Attribute Table (PAT).
 *
 *   ONLY FOR > PENTIUM III PROCESSORS
 */
#define PTE_PAT       7

/*
 * Global (G) flag, bit 8
 *   Indicates a global page when set. When a page is marked global and the PGE
 *   flag of CR4 is set, the PTE or PDE is not invalidated in the TLB on a
 *   task switch or write to CR3.
 *
 *   ONLY FOR > PENTIUM PRO PROCESSORS
 */
#define PDE_G         8
#define PTE_G         8

/*
 * Available, bits 9, 10, 11
 *   These bits are available for use by the system programmer.
 */
#define PDE_AVAIL     9
#define PDE_AVAIL_MASK MASK(3)
#define PTE_AVAIL     9
#define PTE_AVAIL_MASK MASK(3)

/*
 * Page Table Base Address (PT), bits 12-31 (PDE ONLY)
 *   The address of the Page Table for this PDE. Since Page Tables are page 
 *   aligned, we know that the lower 12 bits are 0, and we only needed to 
 *   upper 20 bits to address the Page Table.
 */
#define PDE_PT        12
#define PDE_PT_MASK   MASK(20)

/*
 * Physical Page Address (PP), bits 12-31 (PTE ONLY)
 *   The address of the physical page in memory. Since pages are page-aligned,
 *   the lower 12 bits are 0 for this address.
 */
#define PTE_PP        12
#define PTE_PP_MASK   MASK(20)

#endif /* !__X86_VM_H__ */
