/**
 * @file x86/reg.h
 *
 * @brief Functions and datastructures for manipulating x86 registers.
 *
 */
#ifndef __X86_REG_H__
#define __X86_REG_H__

#include <arch/seg.h>
#include <stdint.h>
#include <kernel/debug.h>

/*
 *
 * WARNING
 *    If you change this struct in any way you must also fix:
 *    - restore_registers (asm.S)
 *    - exn_* (exn_wrappers.S)
 *    - fork_regs (fork.c)
 *
 */
struct __attribute__((__packed__)) registers {
	/*
	 * control registers (cr0 and cr4 are system-level registers)
	 */
	uint32_t cr3;
	uint32_t cr2;

	/* 0x8 */

	/*
	 * general purpose registers (pusha order)
	 */
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	/* 0x24 */

	/*
	 * data segment registers
	 */
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;

	/* 0x34 */

	/*
	 * program registers (iret order)
	 */
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;

	/* 0x48 */
};

#define INIT_REGS \
{ \
	.gs = SEGSEL_USER_DS, \
	.fs = SEGSEL_USER_DS, \
	.es = SEGSEL_USER_DS, \
	.ds = SEGSEL_USER_DS, \
	.ss = SEGSEL_USER_DS, \
	.cs = SEGSEL_USER_CS, \
}

/**
 * @brief Set all the hardware registers to that values in the registers
 * struct.
 */
void restore_registers(struct registers *regs);

/**
 * @brief Set the program counter (instruction pointer) in the current
 * thread's registers (for the next call to restore_registers())
 */
#define set_pc_reg(_regs, _program_counter) \
	do { \
		(_regs)->eip = (_program_counter); \
	} while (0)

/**
 * @brief Set the stack pointer in the current thread's registers (for
 * the next call to restore_registers()).
 */
#define set_sp_reg(_regs, _stack_pointer) \
	do { \
		(_regs)->esp = (_stack_pointer); \
	} while (0)

/**
 * @brief Get the current value of the stack pointer. This reads directly from
 * the hardware register.
 */
uint32_t get_esp();
#define get_sp get_esp

#define DEBUG_REGS(_regs) \
	DEBUG("struct registers %p\n" \
		"cr3:    0x%08x\n" \
		"cr2:    0x%08x\n" \
		"edi:    0x%08x\n" \
		"esi:    0x%08x\n" \
		"ebp:    0x%08x\n" \
		"ebx:    0x%08x\n" \
		"edx:    0x%08x\n" \
		"ecx:    0x%08x\n" \
		"eax:    0x%08x\n" \
		"gs:     0x%08x\n" \
		"fs:     0x%08x\n" \
		"es:     0x%08x\n" \
		"ds:     0x%08x\n" \
		"eip:    0x%08x\n" \
		"cs:     0x%08x\n" \
		"eflags: 0x%08x\n" \
		"esp:    0x%08x\n" \
		"ss:     0x%08x\n" \
		,                  \
		(_regs),           \
		(_regs)->cr3,      \
		(_regs)->cr2,      \
		(_regs)->edi,      \
		(_regs)->esi,      \
		(_regs)->ebp,      \
		(_regs)->ebx,      \
		(_regs)->edx,      \
		(_regs)->ecx,      \
		(_regs)->eax,      \
		(_regs)->gs,       \
		(_regs)->fs,       \
		(_regs)->es,       \
		(_regs)->ds,       \
		(_regs)->eip,      \
		(_regs)->cs,       \
		(_regs)->eflags,   \
		(_regs)->esp,      \
		(_regs)->ss)

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

/*
 * EFLAGS
 *
 * TODO
 */
int32_t get_eflags(void);

#endif /* !__X86_REG_H__ */
