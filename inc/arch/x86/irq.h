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

void __int(uint8_t n);
void __enable_interrupts(void);
void __disable_interrupts(void);

/*
 * Entrypoints for IRQs. These are installed in the IDT.
 */
void __irq0(void);
void __irq1(void);
void __irq2(void);
void __irq3(void);
void __irq4(void);
void __irq5(void);
void __irq6(void);
void __irq7(void);
void __irq8(void);
void __irq9(void);
void __irq10(void);
void __irq11(void);
void __irq12(void);
void __irq13(void);
void __irq14(void);
void __irq15(void);

#endif /* !__X86_IRQ_H__ */
