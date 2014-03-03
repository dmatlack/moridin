/**
 * @file x86/cpu.c
 *
 * @brief General processor level enable/disable functions, low level
 * tweaking, etc.
 *
 */
#include <arch/cpu.h>
#include <arch/reg.h>
#include <arch/seg.h>
#include <stdint.h>
#include <stddef.h>
#include <kernel/debug.h>

#define GENERATE_CRX_SET_BIT_FN(_CRX_) \
  void cr##_CRX_##_set_bit(int index, int bit) { \
    int cr = get_cr##_CRX_(); \
    set_bit(&cr, index, bit); \
    set_cr##_CRX_(cr); \
  }
GENERATE_CRX_SET_BIT_FN(0)
GENERATE_CRX_SET_BIT_FN(4)

void enable_paging(void) {
  TRACE();
  cr0_set_bit(CR0_PG, 1);
}

void disable_paging(void) {
  TRACE();
  cr0_set_bit(CR0_PG, 0);
}

void enable_protected_mode(void) {
  TRACE();
  cr0_set_bit(CR0_PE, 1);
}

void enable_real_mode(void) {
  TRACE();
  cr0_set_bit(CR0_PE, 0);
}

void disable_fpu(void) {
  TRACE();
  cr0_set_bit(CR0_EM, 1);
}

void enable_global_pages(void) {
  TRACE();
  cr4_set_bit(CR4_PGE, 1);
}

void enable_write_protect(void) {
  TRACE();
  cr0_set_bit(CR0_WP, 1);
}

/**
 * @brief Jump to user space with default register values.
 *
 * @param kstack The location of the top of the kernel stack to use on interrupts
 * @param page_dir The page directory to use.
 * @param entry The address of the first instruction to execute
 * @param ustack The location of the top of the user runtime stack to use.
 */
void iret_to_userspace(uint32_t kstack, uint32_t page_dir,
                       uint32_t entry, uint32_t ustack) {
  struct registers regs;

  set_esp0(kstack);

  regs.cr4 = get_cr4();
  regs.cr3 = page_dir;
  regs.cr2 = 0;
  regs.cr0 = get_cr0();

  regs.edi = 0;
  regs.esi = 0;
  regs.ebp = 0;
  regs.ebx = 0;
  regs.edx = 0;
  regs.ecx = 0;
  regs.eax = 0;
  
  regs.gs = SEGSEL_USER_DS;
  regs.fs = SEGSEL_USER_DS;
  regs.es = SEGSEL_USER_DS;
  regs.ds = SEGSEL_USER_DS;
 
  regs.eip    = entry;
  regs.cs     = SEGSEL_USER_CS;
  regs.eflags = get_eflags();
  regs.esp    = ustack;
  regs.ss     = SEGSEL_USER_DS;

  restore_registers(&regs);
}
