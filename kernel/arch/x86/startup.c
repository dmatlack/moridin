/**
 * @file x86/startup.c
 */
#include <arch/cpu.h>
#include <arch/reg.h>
#include <arch/exn.h>
#include <arch/idt.h>
#include <arch/irq.h>
#include <dev/serial/8250.h>
#include <kernel/debug.h>
#include <assert.h>
#include <boot/multiboot.h>

extern void __syscall_entry(void);

extern char kernel_idt[];
extern char kernel_gdt[];
extern char kernel_tss[];
extern char boot_stack_top[];
extern char boot_stack_bottom[];
extern char boot_page_dir[];

void invalid_interrupt(void) { panic("INVALID INTERRUPT OCCURRED"); }

/* x86 startup routines */
extern void init_8253(void);

void arch_startup(void)
{
	int vector;

	/* Try to begin logging to the serial console. */
	early_init_8250();
	early_log_init(early_i8250_putchar, CONFIG_LOG_LEVEL);

	ASSERT_EQUALS((size_t) kernel_idt, 0x10000c);
	ASSERT_EQUALS((size_t) kernel_gdt, 0x10080c);
	ASSERT_EQUALS((size_t) kernel_tss, 0x10083c);

	multiboot_init();

	/*
	 * Print out some symbols defined in arch/boot.S
	 */
	INFO("boot_stack:    0x%08x, 0x%08x", boot_stack_bottom, boot_stack_top);
	INFO("boot_page_dir: 0x%08x", boot_page_dir);

	disable_fpu();

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

	/*
	 * Install the global system call handler.
	 */
	idt_syscall_gate(0x80, __syscall_entry);

	/*
	 * Initialize the interrupt controller and associated handlers.
	 */
	pic_irq_init();

	/* COM Ports */
	init_8250();

	/* Programmable Interval Timer */
	init_8253();
}
