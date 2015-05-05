/**
 * @file x86/irq.h
 *
 */
#ifndef __X86_IRQ_H__
#define __X86_IRQ_H__

#include <arch/idt.h>
#include <arch/pic.h>
#include <arch/reg.h>

#include <kernel/irq.h>
#include <list.h>

/*
 *
 * x86 Hardware Interrupts
 * 
 * 8259A Input pin  Interrupt Number  Description
 * ------------------------------------------------------------------
 * IRQ0             0x08              Timer
 * IRQ1             0x09              Keyboard
 * IRQ2             0x0A              Cascade for 8259A Slave controller
 * IRQ3             0x0B              Serial port 2
 * IRQ4             0x0C              Serial port 1
 * IRQ5             0x0D              AT systems: Parallel Port 2. PS/2 systems: reserved
 * IRQ6             0x0E              Diskette drive
 * IRQ7             0x0F              Parallel Port 1
 * IRQ8/IRQ0        0x70              CMOS Real time clock
 * IRQ9/IRQ1        0x71              CGA vertical retrace
 * IRQ10/IRQ2       0x72              Reserved
 * IRQ11/IRQ3       0x73              Reserved
 * IRQ12/IRQ4       0x74              AT systems: reserved. PS/2: auxiliary device
 * IRQ13/IRQ5       0x75              FPU
 * IRQ14/IRQ6       0x76              Hard disk controller
 * IRQ15/IRQ7       0x77              Reserved
 *
 */
#define MAX_NUM_IRQS       16
#define IRQ_TIMER           0
#define IRQ_KEYBOARD        1
#define IRQ_SERIAL2         3
#define IRQ_SERIAL1         4

void pic_irq_init(void);

#define ack_irq(irq) pic_eoi(irq)

void generate_irq(int irq);

#define cli() __asm__ __volatile__("cli");
#define sti() __asm__ __volatile__("sti");

#define disable_irqs() cli()
#define enable_irqs() sti()

static inline void disable_save_irqs(unsigned long *flags)
{
	*flags = get_eflags() & 0x200;
	disable_irqs();
}

static inline void restore_irqs(unsigned long flags)
{
	if (flags & 0x200)
		enable_irqs();
}

static inline bool irqs_enabled(void)
{
	return get_eflags() & 0x200;
}

#endif /* !__X86_IRQ_H__ */
