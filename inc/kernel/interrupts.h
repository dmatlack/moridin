/**
 * @file interrupts.h
 *
 * @brief Functions and defines for the interrupt system.
 *
 * @author David Matlack
 */
#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

/*
 * The following defines how our interrupt handlers will be layed out 
 * in the Interrupt Descriptor Table (IDT).
 */
#define INTERRUPTS_IDT_EXN_OFFSET        0x0    // exception handlers
#define INTERRUPTS_IDT_PIC_MASTER_OFFSET 0x20   // hardware devices
#define INTERRUPTS_IDT_PIC_SLAVE_OFFSET  0x28   // hardware devices
#define INTERRUPTS_IDT_SYSCALL_OFFSET    0x40   // system calls

/**
 * @brief Initialize the entires interrupt subsytem by installing all
 * of the necessary handlers in the Interrupt Descriptor Table (IDT).
 */
int interrupts_init(void);

#endif /* __INTERRUPTS_H__ */
