/**
 * @file x86/cpu.c
 *
 * @brief General processor level enable/disable functions, low level
 * tweaking, etc.
 *
 * @author David Matlack
 */
#include <x86/cpu.h>
#include <x86/reg.h>
#include <stdint.h>
#include <stddef.h>

#define GENERATE_CRX_SET_BIT_FN(_CRX_) \
  void cr##_CRX_##_set_bit(int index, int bit) { \
    int cr = get_cr##_CRX_(); \
    cr = SET_BIT(cr, index, bit); \
    set_cr##_CRX_(cr); \
  }
GENERATE_CRX_SET_BIT_FN(0)
GENERATE_CRX_SET_BIT_FN(4)

void x86_enable_paging(void) {
  cr0_set_bit(CR0_PG, 1);
}

void x86_disable_paging(void) {
  cr0_set_bit(CR0_PG, 0);
}

void x86_enable_protected_mode(void) {
  cr0_set_bit(CR0_PE, 1);
}

void x86_enable_real_mode(void) {
  cr0_set_bit(CR0_PE, 0);
}

void x86_disable_fpu(void) {
  cr0_set_bit(CR0_EM, 1);
}

void x86_enable_global_pages(void) {
  cr4_set_bit(CR4_PGE, 1);
}

