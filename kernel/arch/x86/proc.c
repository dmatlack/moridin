/**
 * @file arch/x86/proc.c
 */
#include <kernel/proc.h>

void set_thread_regs(struct registers *regs) {
  CURRENT_THREAD->regs = regs;
}
