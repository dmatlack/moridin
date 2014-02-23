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
#include <arch/x86/seg.h>
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

/**
 * @brief Jump to user space with default register values.
 *
 * @param kstack The location of the top of the kernel stack to use on
 *    interrupts
 * @param page_dir The page directory to use.
 * @param entry The address of the first instruction to execute
 * @param ustack The location of the top of the user runtime stack to use.
 */
void iret_to_userspace(uint32_t kstack, uint32_t page_dir,
                       uint32_t entry, uint32_t ustack) {
  set_esp0(kstack);

  restore_registers(
      get_cr4(),                    // cr4
      page_dir,                     // cr3
      0,                            // cr2
      get_cr0(),                    // cr0

      0,                            // edi
      0,                            // esi
      0,                            // ebp
      0,                            // ignore
      0,                            // ebx
      0,                            // edx
      0,                            // ecx
      0,                            // eax

      SEGSEL_USER_DS,               // gs
      SEGSEL_USER_DS,               // fs
      SEGSEL_USER_DS,               // es
      SEGSEL_USER_DS,               // ds

      entry,                        // eip
      SEGSEL_USER_CS,               // cs
      get_eflags(),                 // eflags
      ustack,                       // esp
      SEGSEL_USER_DS                // ss
  );
}
