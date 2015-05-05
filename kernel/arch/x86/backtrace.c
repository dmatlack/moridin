/**
 * @file arch/x86/backtrace.c
 */
#include <kernel/proc.h>
#include <kernel/symbols.h>
#include <kernel/log.h>
#include <arch/reg.h>

extern char boot_stack_bottom[];
extern char boot_stack_top[];

static inline bool on_boot_stack(void)
{
	char *sp = (void *) get_sp();

	return (boot_stack_bottom <= sp && sp < boot_stack_top);
}

void backtrace(void)
{
	unsigned long *stack_start, *stack_end;
	unsigned long *bp = (void *) get_ebp();

	if (on_boot_stack()) {
		stack_start = (void *) boot_stack_bottom;
		stack_end = (void *) boot_stack_top;
		INFO("early boot");
	} else {
		stack_start = (void *) KSTACK_START;
		stack_end = (void *) KSTACK_END;
		INFO("task %d:%d", CURRENT_PROCESS->pid, CURRENT_THREAD->tid);
	}

	INFO("stack [0x%08x, 0x%08x]", (unsigned long) stack_start,
				       (unsigned long) stack_end);

	while (stack_start <= bp && bp < stack_end) {
		unsigned long return_address = *(bp + 1);
		struct symbol *symbol = resolve_symbol(return_address);

		if (symbol) {
			INFO("    0x%08x    %-30s+0x%x",
			     return_address, symbol->name,
			     return_address - symbol->address);
		} else
			INFO("    0x%08x    ???", return_address);


		bp = (void *) *bp;
	}
}
