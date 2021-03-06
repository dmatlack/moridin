/**
 * @file x86/exn.c
 *
 */
#include <arch/exn.h>
#include <arch/idt.h>
#include <arch/reg.h>
#include <arch/vm.h>

#include <assert.h>
#include <types.h>

void page_fault(int vector, int error, struct registers *regs);

/*
 * x86 exception handling jump table
 */
void (*exn_table[])(int vector, int error, struct registers *regs) =  {
	[0 ... 13]  = exn_panic,
	[14]        = page_fault,
	[15 ... 20] = exn_panic
};

void exn_panic(int vector, int error, struct registers *regs)
{
	struct x86_exn *exn = &x86_exceptions[vector];

	ERROR("\n"
	"-------------------------------------------------------------------\n"
	"%d %s %s (cause: %s)\n"
	"-------------------------------------------------------------------\n"
	"eip: 0x%08x\n"
	"ebp: 0x%08x\n"
	"\n"
	"edi: 0x%08x esi: 0x%08x\n"
	"eax: 0x%08x ebx: 0x%08x\n"
	"ecx: 0x%08x edx: 0x%08x\n"
	"\n"
	"cr0: 0x%08x\n"
	"cr2: 0x%08x\n"
	"cr3: 0x%08x\n"
	"cr4: 0x%08x\n"
	"\n"
	"ds: 0x%08x\n"
	"es: 0x%08x\n"
	"fs: 0x%08x\n"
	"gs: 0x%08x\n"
	"\n"
	"error: %d\n"
	"-------------------------------------------------------------------",
	exn->vector, exn->mnemonic, exn->description, exn->cause,
	regs->eip,
	regs->ebp,
	regs->edi, regs->esi,
	regs->eax, regs->ebx,
	regs->ecx, regs->edx,
	get_cr0(),
	regs->cr2,
	regs->cr3,
	get_cr4(),
	regs->ds,
	regs->es,
	regs->fs,
	regs->gs,
	error
	);
	panic("Exception %d during boot. Aborting.", exn->vector);
}

/**
 * @brief These are all the exceptions that can be generated by an x86
 * processor.
 */
struct x86_exn x86_exceptions[] = {
	{ 0, "#DE", "Divide Error Fault", 
		X86_FAULT, 
		"DIV or IDIV instructions", 
		false,
		EXN_HANDLER_NAME(0)
	},
	{ 1, "#DB", "Debug", 
		X86_FAULT | X86_TRAP, 
		"INT 1 instruction", 
		false,
		EXN_HANDLER_NAME(1)
	},
	{ 2, "NMI", "Non-Maskable Interrupt", 
		0, 
		"Nonmaskable external interrupt", 
		false,
		EXN_HANDLER_NAME(2)
	},
	{ 3, "#BP", "Breakpoint", 
		X86_TRAP, 
		"INT 3 instruction", 
		false,
		EXN_HANDLER_NAME(3)
	},
	{ 4, "#OF", "Overflow", 
		X86_TRAP, 
		"INTO instruction", 
		false,
		EXN_HANDLER_NAME(4)
	},
	{ 5, "#BR", "BOUND Range Exceeded", 
		X86_FAULT, 
		"BOUND instruction", 
		false,
		EXN_HANDLER_NAME(5)
	},
	{ 6, "#UD", "Invalid Opcode", 
		X86_FAULT, 
		"UD2 instruction or reserved opcode", 
		false,
		EXN_HANDLER_NAME(6)
	},
	{ 7, "#NM", "Device Not Available (No Math Coprocessor)", 
		X86_FAULT, 
		"Floating-point or WAIT/FWAIT instruction", 
		false,
		EXN_HANDLER_NAME(7)
	},
	{ 8, "#DF", "Double Fault", 
		X86_ABORT, 
		"Any instruction that can generate an exception, an NMI, "
			"or an INTR",
		true,
		EXN_HANDLER_NAME(8)
	},
	{ 9, "---", "Coprocessor Segment Overrun (reserved)", 
		X86_FAULT, 
		"Floating-point instruction", 
		false,
		EXN_HANDLER_NAME(9)
	},
	{ 10, "#TS", "Invalid TSS", 
		X86_FAULT, 
		"Task switch or TSS access", 
		true,
		EXN_HANDLER_NAME(10)
	},
	{ 11, "#NP", "Segment Not Present", 
		X86_FAULT, 
		"Loading segment registers or accessing system segments.",
		true,
		EXN_HANDLER_NAME(11)
	},
	{ 12, "#SS", "Stack-Segment Fault", 
		X86_FAULT, 
		"Stack operations and SS register", 
		true,
		EXN_HANDLER_NAME(12)
	},
	{ 13, "#GP", "General Protection Fault", 
		X86_FAULT, 
		"Any memory reference and other protection checks", 
		true,
		EXN_HANDLER_NAME(13)
	},
	{ 14, "#PF", "Page Fault", 
		X86_FAULT, 
		"Any memory reference", 
		true,
		EXN_HANDLER_NAME(14)
	},
	{ 15, "---", "Intel reserved", 
		0, 
		"Do not use", 
		false,
		EXN_HANDLER_NAME(15)
	},
	{ 16, "#MF", "x87 FPU Floating-Point Error (Math Fault)", 
		X86_FAULT, 
		"x87 FPU floating-point or WAIT/FWAIT instruction", 
		false,
		EXN_HANDLER_NAME(16)
	},
	{ 17, "#AC", "Alignment Check", 
		X86_FAULT, 
		"Any data reference in memory", 
		true,
		EXN_HANDLER_NAME(17)
	},
	{ 18, "#MC", "Machine Check", 
		X86_ABORT, 
		"Error codes (if any) and source are model dependent", 
		false,
		EXN_HANDLER_NAME(18)
	},
	{ 19, "#XF", "SIMD Floating-Point Exception", 
		X86_FAULT,
		"SSE and SSE2 floating-point instructions", 
		false,
		EXN_HANDLER_NAME(19)
	}

	/*
	 * Intel reserves vectors 20-31
	 */

	/*
	 * User defined vectors 32-255
	 */
};
