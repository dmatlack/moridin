/**
 * @file x86/cpu.c
 *
 * @brief General processor level enable/disable functions, low level
 * tweaking, etc.
 *
 * @author David Matlack
 */
#include <arch/x86/cpu.h>
#include <arch/x86/reg.h>
#include <stdint.h>
#include <stddef.h>
#include <debug.h>

#define GENERATE_CRX_SET_BIT_FN(_CRX_) \
  void cr##_CRX_##_set_bit(int index, int bit) { \
    int cr = get_cr##_CRX_(); \
    set_bit(&cr, index, bit); \
    set_cr##_CRX_(cr); \
  }
GENERATE_CRX_SET_BIT_FN(0)
GENERATE_CRX_SET_BIT_FN(4)

void x86_enable_paging(void) {
  TRACE();
  cr0_set_bit(CR0_PG, 1);
}

void x86_disable_paging(void) {
  TRACE();
  cr0_set_bit(CR0_PG, 0);
}

void x86_enable_protected_mode(void) {
  TRACE();
  cr0_set_bit(CR0_PE, 1);
}

void x86_enable_real_mode(void) {
  TRACE();
  cr0_set_bit(CR0_PE, 0);
}

void x86_disable_fpu(void) {
  TRACE();
  cr0_set_bit(CR0_EM, 1);
}

void x86_enable_global_pages(void) {
  TRACE();
  cr4_set_bit(CR4_PGE, 1);
}

void x86_enable_write_protect(void) {
  TRACE();
  cr0_set_bit(CR0_WP, 1);
}
