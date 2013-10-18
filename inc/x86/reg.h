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


#endif /* !__X86_REG_H__ */
