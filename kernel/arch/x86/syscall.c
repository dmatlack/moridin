/**
 * @file arch/x86/syscall.c
 */
#include <kernel/proc.h>
#include <arch/cpu.h>

//TODO: move function this somewhere else
void return_from_syscall(int ret) {
  CURRENT_THREAD->regs->eax = ret;

  set_esp0(KSTACK_TOP);

  restore_registers(CURRENT_THREAD->regs);
}
