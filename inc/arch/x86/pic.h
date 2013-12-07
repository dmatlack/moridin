/**
 * @file pic.h
 *
 * @brief Defines for the Programmable Interrupt Controller (PIC).
 *
 * The Programmable Interrupt Controller (PIC, also called 8259), manages
 * all hardware interrupts and sends them to the appropriate software 
 * interrupts. This keeps the CPU from having to poll devices to ask if they
 * have any interrupts to deliver.
 *
 * The PIC serializes interrupts (delivers them one at a time, in an order).
 * Interrupt Requests (IRQ), also known as Hardware Interrupts, are generated 
 * by external devices. They are delivered to the PIC which will then 
 * interrupt what is running on the CPU and start running a predefined 
 * interrupt handler.
 *
 * Software Interrupts (INT) are generated by software instructions (on x86,
 * this is the INT intstruction).
 *
 * There are 2 PICs on modern x86 systems. Each PIC can connect to up to 8
 * devices each. One PIC is used as a master and one as a slave, so the slave
 * actually counts toward one of the devices leaving 15 available spots.
 *
 * reference: http://wiki.osdev.org/PIC
 *  (most of the documentation on this page was taken from this wiki, for
 *   my own benifit, typing it out to learn it)
 *
 * @author David Matlack
 */
#ifndef __ARCH_X86_PIC_H__
#define __ARCH_X86_PIC_H__

#include <stdint.h>

/**
 * Each PIC chip has a command port and a data port. When no command is given, 
 * the data port allows us to access the inturrupt mask of the 8259 chip.
 */
#define PIC_MASTER_CMD  0x0020
#define PIC_MASTER_DATA 0x0021

#define PIC_SLAVE_CMD  0x00A0
#define PIC_SLAVE_DATA 0x00A1

/**
 * Default Mapping of IRQs to Interrupt Vectors:
 *
 * Chip   | IRQ       | Vector Offset | Interrupt Number
 * -------+-----------+---------------+-----------------
 * Master | 0 to 7    | 0x08          | 0x08 to 0x0F
 * Slave  | 8 to 15   | 0x70          | 0x70 to 0x77
 */

/**
 * End Of Interrupt
 *
 * This is a command sent to PICs at the end of an interrupt routine.
 * 
 * If the interrupt came from the Master PIC, you must send an EOI to just 
 * the master. If the interrupt came from the slave you must send an EOI
 * to BOTH the master and the slave.
 */
#define PIC_EOI 0x20

/**
 * PIC Initialization
 *
 * The initialization command tells the PIC that it should wait for 3 
 * data writes:
 *  1. Tell it t's vector offset (ICW2).
 *  2. Tell it how it's wired to master/slave (ICW3).
 *  3. Give it additional information about its environment (ICW4).
 */
#define PIC_INIT 0x11

// 1. Vector Offset
//TODO do you want to remap the PIC

// 2. Master/Slave wiring
#define PIC_ICW3_TELL_MASTER_ABOUT_SLAVE  2 //???
#define PIC_ICW3_TELL_SLAVE_CASCADE_ID    4 //???

// 3. Environment
#define PIC_ICW4_8086 0x01

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
#define PIC_IRQ_COUNT      16
#define PIC_IRQ_TIMER      0
#define PIC_IRQ_KEYBOARD   1
#define PIC_IRQ_SERIAL2    3
#define PIC_IRQ_SERIAL1    4


/**
 * @brief Initialize the PIC.
 */
int pic_init(void);

/**
 * @brief Remap the Master and Slave PIC's Vector Offsets.
 */
void pic_remap(uint32_t master_offset, uint32_t slave_offset);

/**
 * Interrupt Mask Register (IMR): A bitmap of the request lines going into 
 * the PIC. When a bit is set, the PIC ignores the request and continues 
 * operation.
 */

/** @brief Set a bit (0-15) in the Interrupt Mask Register (IMR). */
void pic_imr_set(uint8_t irq);
/** @brief Clear a bit (0-15) in the Interrupt Mask Register (IMR). */
void pic_imr_clear(uint8_t irq);

/**
 * PIC Interrupt Status Registers: ISR, IRR
 *
 * In-Service Register (ISR): Tells which interrupts are being serviced.
 *
 * Interrupt Request Register (IRR): Tells which interrupts are being 
 * raised.
 *
 * The ISR and IRR are each 8 bits, and there is one of each for the master
 * and slave.
 */
#define PIC_READ_ISR 0x0a
#define PIC_READ_IRR 0x0b

/** @brief Return the PICs' In-Service Register */
uint16_t pic_get_isr(void);
/** @brief Return the PICs' Interrupt Request Register */
uint16_t pic_get_irr(void);

/** @brief Send an "END OF INTERRUPT" command to the PIC */
void pic_eoi(uint8_t irq);

#endif /* !__ARCH_X86_PIC_H__ */
