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
#include <arch/vm.h>

#include <stdint.h>
#include <stddef.h>
#include <kernel/debug.h>

#include <kernel/proc.h>

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

void jump_to_userspace(void) {
  struct registers *regs = &CURRENT_THREAD->regs;

  set_esp0(KSTACK_TOP);

  regs->cr3 = __phys(CURRENT_PAGE_DIR);
  regs->cr2 = 0;
  regs->eflags = get_eflags();

  restore_registers(regs);
}
