/**
 * @file arch/x86/proc.c
 */
#include <kernel/proc.h>
#include <arch/cpu.h>

void set_thread_regs(struct registers *regs) {
  CURRENT_THREAD->regs = regs;
}

//TODO: move function this somewhere else
void return_from_syscall(int ret) {
  CURRENT_THREAD->regs->eax = ret;

  set_esp0(KSTACK_TOP);

  //
  // FIXME: this sets cr3 (tlb global flush) every time we return from
  // a syscall. this is bad!
  //
  // (the fix is to just check in POP_REGISTERS if new_cr3 == current_cr3)
  //
  restore_registers(CURRENT_THREAD->regs);
}
