/**
 * @file x86/irq.h
 *
 * @author David Matlack
 */
#ifndef __X86_IRQ_H__
#define __X86_IRQ_H__

#include <x86/idt.h>

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
#define IRQ_TIMER         (IDT_PIC_MASTER_OFFSET + 0x0)
#define IRQ_KEYBOARD      (IDT_PIC_MASTER_OFFSET + 0x1)

void generate_irq(uint8_t irq);

#endif /* !__X86_IRQ_H__ */
