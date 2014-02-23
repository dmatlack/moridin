/**
 * @file x86/irq.h
 *
 * @author David Matlack
 */
#ifndef __X86_IRQ_H__
#define __X86_IRQ_H__

#include <kernel/irq.h>
#include <arch/x86/idt.h>
#include <list.h>

int  x86_init_irq(struct machine_irq_info *info);
void x86_generate_irq(int irq);
void x86_handle_irq(int irq);
void x86_acknowledge_irq(int irq);
void x86_enable_irqs(void);
void x86_disable_irqs(void);

/*
 * Invoke the assembly instruction "int $n"
 */
void __int(uint8_t n);

void __enable_interrupts(void);
void __disable_interrupts(void);

/*
 * Entrypoints for IRQs. These are installed in the IDT.
 */
void irq_0(void);
void irq_1(void);
void irq_2(void);
void irq_3(void);
void irq_4(void);
void irq_5(void);
void irq_6(void);
void irq_7(void);
void irq_8(void);
void irq_9(void);
void irq_10(void);
void irq_11(void);
void irq_12(void);
void irq_13(void);
void irq_14(void);
void irq_15(void);

#endif /* !__X86_IRQ_H__ */
