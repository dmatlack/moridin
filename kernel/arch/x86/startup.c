/**
 * @file x86/startup.c
 */
#include <arch/x86/cpu.h>
#include <arch/x86/reg.h>
#include <arch/x86/exn.h>
#include <arch/x86/idt.h>

#include <debug.h>
#include <assert.h>

extern void x86_syscall(void);

extern char kernel_idt[];
extern char kernel_gdt[];
extern char kernel_tss[];
extern char boot_stack_top[];
extern char boot_stack_bottom[];
extern char boot_page_dir[];

void invalid_interrupt(void) { panic("INVALID INTERRUPT OCCURRED"); }

/**
 * @brief This function initializes most parts of the x86 system.
 */
void arch_startup(void) {
  int vector;

  ASSERT_EQUALS((size_t) kernel_idt, 0x10000c);
  ASSERT_EQUALS((size_t) kernel_gdt, 0x10080c);
  ASSERT_EQUALS((size_t) kernel_tss, 0x10083c);

  /*
   * Print out some symbols defined in arch/x86/boot.S
   */
  kprintf("boot_stack:    0x%08x, 0x%08x\n", boot_stack_bottom, boot_stack_top);
  kprintf("boot_page_dir: 0x%08x\n", boot_page_dir);

  x86_disable_fpu();

  /*
   * Install default handlers for all IDT entries so we panic before 
   * we triple fault.
   */
  for (vector = 0; vector < 256; vector++) {
    idt_exn_gate(vector, invalid_interrupt);
  }

  /*
   * Install handlers in the IDT for each exception type.
   */
  for (vector = 0; vector < X86_NUM_EXCEPTIONS; vector++) {
    idt_exn_gate(vector, x86_exceptions[vector].handler);
  }
  x86_exn_set_handler(x86_exn_panic);

  /*
   * Install the global system call handler.
   */
  idt_syscall_gate(0x80, x86_syscall);
}
