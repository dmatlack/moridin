/**
 * @file arch/x86/syscall.c
 */
#include <kernel/proc.h>
#include <arch/cpu.h>
#include <arch/vm.h>

//TODO: move function this somewhere else
void return_from_syscall(int ret)
{
	TRACE("ret=0x%x", ret);

	CURRENT_THREAD->regs->eax = ret;

	// FIXME: we will need to set this when returning from all interrupts
	// and exceptions since eventually they all can result in a task switch
	// (e.g. timer firing switches tasks, or keyboard interrupt causes task
	// switch to the process waiting on IO).
	set_esp0(KSTACK_TOP);

	restore_registers(CURRENT_THREAD->regs);
}
