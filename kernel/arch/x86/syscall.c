/**
 * @file arch/x86/syscall.c
 */
#include <kernel/proc.h>
#include <arch/cpu.h>
#include <arch/vm.h>
#include <arch/syscall.h>

void return_from_syscall(int ret)
{
	struct thread *current = CURRENT_THREAD;

	TRACE("ret=0x%x", ret);

	current->regs->eax = ret;
	restore_registers(current->regs);
}

void arch_sched_switch_end(void)
{
	set_esp0(KSTACK_TOP);
}

void jump_to_userspace(void)
{
	struct registers *regs = CURRENT_THREAD->regs;

	set_esp0(KSTACK_TOP);

	regs->cr3 = __phys(CURRENT_PAGE_DIR);
	regs->cr2 = 0;
	regs->eflags = get_eflags() | 0x200; /* enable interrupts */

	return_from_syscall(0);
}
