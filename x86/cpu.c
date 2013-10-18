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

void cr0_set_bit(int index, int bit) {
  int cr0 = get_cr0();
  cr0 = SET_BIT(cr0, index, bit);
  set_cr0(cr0);
}

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
