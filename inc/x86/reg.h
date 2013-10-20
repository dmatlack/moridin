/**
 * @file x86/reg.h
 *
 * @brief Functions and datastructures for manipulating x86 registers.
 *
 * @author David Matlack
 */
#ifndef __X86_REG_H__
#define __X86_REG_H__

#include <stdint.h>

struct __attribute__ ((__packed__)) x86_pusha_stack {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t ignore;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
};

struct __attribute__ ((__packed__)) x86_iret_stack {
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t esp;
  uint32_t ss;
};

/*
 *
 * Control Registers
 * ----------------------------------------------------------------------------
 * Intel System Programming Guide (Section 2.5)
 *
 * cr0 - Contains system control flags that control operating mode and
 *       states of the preocessor.
 *
 * cr1 - Reserved (aka yay we don't have to care)
 *
 * cr2 - Contains the linear address that caused the page fault (when one
 *       occurs).
 * 
 * cr3 - Contains the *physical address* of the page directory, and two
 *       flags. This register is also know as the Page Directory Base
 *       Register (PGBR).
 *
 *       Only the highest 20 bits are used for the page directory address
 *       (the low twelve bytes are assumed to be 0; thus page directories
 *       are 4 KB aligned).
 *
 *       Bit 3: PWT, Bit 4: PCD
 *          These bits (read: flags) control the caching of the page 
 *          directory in the processor's internal caches (not the TLB).
 *
 * cr4 - Contains a group of flags that enable several architectural 
 *       extensions, and indicate os support for specific processor 
 *       capabilities.
 * ----------------------------------------------------------------------------
 *
 */

/*
 * CR0: System Control Flags
 *
 *  Name | Bit | Function when set (1)
 * ------+-----+----------------------------------------------------------------
 *  PG   | 31  | Enable paging
 *  CD   | 30  | Disable caching
 *  NW   | 29  | Not Write-through
 *  AM   | 18  | Enable automatic alignment checking
 *  WP   | 16  | Inhibits supervisor level procedures from writing into user
 *               level read-only pages.
 *  NE   | 5   | Enables native mechanism for reporting x87 FPU errors
 *  TS   | 3   | Enables saving FPU/MMX/SSE/SSE2 during task switch
 *  EM   | 2   | Indicates cpu does not have an x87 FPU
 *  MP   | 1   | Control interaction of WAIT instruction with the TS flag
 *  PE   | 0   | Enables protected mode (32-bit) when set
 *
 */
#define CR0_PG 31
#define CR0_CD 30
#define CR0_NW 29
#define CR0_AM 18
#define CR0_WP 16
#define CR0_NE 5
#define CR0_TS 3
#define CR0_EM 2
#define CR0_MP 1
#define CR0_PE 0
int32_t get_cr0(void);
void set_cr0(int32_t);

/*
 * CR2: Address that caused the page fault.
 */
int32_t get_cr2(void);
void set_cr2(int32_t);

/*
 * CR3: The physical address of the page directory.
 *
 *  Name | Bit | Function when set (1)
 * ------+-----+----------------------------------------------------------------
 *  PCD  | 4   | Disable caching (L1 and L2) of the page directory
 *  PWT  | 3   | Write-through caching is enabled (write-back enabled if not set)
 *
 */
#define CR3_PCD 4
#define CR3_PWT 3
int32_t get_cr3(void);
void set_cr3(int32_t);

/*
 * CR4: Architecture Control Flags
 *
 *  Name | Bit | Function when set (1)
 * ------+-----+----------------------------------------------------------------
 *  VME  | 0   | Enables interrupt and exception handling extensions in virtual
 *               8086 mode
 *  PVI  | 1   | Enables hardware support for a virtual interrupt flag (VIF) in
 *               protected mode
 *  TSD  | 2   | Restricts the RTDSC instruction to procedures running at PL0
 *  DE   | 3   | References to debug registers DR4 and DR5 cause an unidentified
 *               opcode exception (#UD) to be generated
 *  PSE  | 4   | Enables 4MB pages when set. (4KB pages if not set)
 *  PAE  | 5   | Enables paging mechanism to reference 36-bit addresses 
 *  MCE  | 6   | Enables the machine-check exception
 *  PGE  | 7   | Enables the global page feature, which allows frequently used
 *               or shared pages to be marked as global to all users (must set
 *               the global flag in page-directory and page-table entries).
 *               Global pages are not flushed from the TLB.
 *  PCE  | 8   | Enables execution of the RDPMC instruction for programs running
 *               at any priveledge level
 *
 *  OSFXSR     | 9  | OS support for FXSAVE and FXRSTOR instructions
 *  OSXMMEXCPT | 10 | OS support unmasked SIMD floating point exceptions
 *
 */
#define CR4_VME       0
#define CR4_PVI       1
#define CR4_TSD       2
#define CR4_DE        3
#define CR4_PSE       4
#define CR4_PAE       5
#define CR4_MCE       6
#define CR4_PGE       7
#define CR4_PCE       8
#define CR4_OSFXSR    9
#define CR4_OSMMEXCPT 10
int32_t get_cr4(void);
void set_cr4(int32_t);

#endif /* !__X86_REG_H__ */
