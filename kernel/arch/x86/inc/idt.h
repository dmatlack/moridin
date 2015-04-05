/**
 * @file idt.h
 *
 * @brief Defines for the x86 Interrupt Descriptor Table.
 *
 * The Interrupt Descriptor Table (IDT) is contiguous chunk of memory
 * that holds Interrupt Gates. The BASE (address) and LIMIT (number of 
 * bytes) of the IDT is stored in the IDTR register.
 *
 * The IDT is very similar to the GDT, with the following exceptions:
 *  - The first entry (0) of the IDT can be used.
 *  - There are 256 interrupts, so the IDT should have 256 entries
 *    (256d = 0x100).
 *  - The IDT techinically can contain more or less than 256 entries.
 *    More entries are ignored. If there is not an entry for when an
 *    interrupt comes in, a General Protection Fault will be raised.
 *
 * Types of IDT Entries
 *    1. Interrupt Gate: Used to specify an Interrupt Service Routine
 *       (a function that will run when the interrupt happens).
 *
 *    2. Trap Gate: Also used to specify an ISR. They are superficially
 *       the same as Interrupt Gates, with one difference. Trap Gates
 *       DISABLE INTERRUPTS upon entry (whereas Interrupt Gates do not).
 *
 *    3. Task Gate: Used to switch tasks. Hardware Multitasking is not
 *       used in this kernel, so we can ignore this.
 *
 * IDT Entries 0 through 32 (0x0 to 0x20) are reserved by Intel for
 * exceptions. Entries 32 to 255 are available for use by PIC IRQ
 * handlers and system call handlers.
 *
 * reference: http://wiki.osdev.org/IDT
 *
 */
#ifndef __IDT_H__
#define __IDT_H__
#include <stdint.h>

#define IDT_GATE_SIZE 8 // bytes (64-bits)

/* DPL (Priveledge Level) */
#define IDT_PL0 0
#define IDT_PL1 1
#define IDT_PL2 2
#define IDT_PL3 3

/* P (Present Flag) */
#define IDT_GATE_PRESENT 1
#define IDT_GATE_ABSENT  0

/* D (16-bit or 32-bit mode?) */
#define IDT_D_32 1
#define IDT_D_16 0

/* Gate Type (Trap or Interrupt Gate) */
#define IDT_GATE_TYPE_TRAP 1
#define IDT_GATE_TYPE_INT  0

/*
 * The following defines how our interrupt handlers will be layed out 
 * in the Interrupt Descriptor Table (IDT).
 */
#define IDT_EXN_OFFSET        0x0    // exception handlers
#define IDT_PIC_MASTER_OFFSET 0x20   // hardware devices (PIC 1)
#define IDT_PIC_SLAVE_OFFSET  0x28   // hardware devices (PIC 2)
#define IDT_SYSCALL_OFFSET    0x40   // system calls

typedef struct {
	uint32_t segsel;
	uint32_t offset;
	uint8_t  p;
	uint8_t  dpl;
	uint8_t  d;
	uint8_t  type;
} idt_gate_t;

/**
 * @brief Install a new gate in the Interrupt Descriptor Table.
 *
 * @param index     The index into the IDT at which to add the new gate
 *                  descriptor will be added.
 * @param segsel    The segment selector wher the interrupt handler is located.
 * @param offset    The offset into the segsel where the interrupt handler
 *                  is located.
 * @param p         Whether or not the gate is "present".
 * @param dpl       The descriptor privelege level.
 * @param d         The size of the gate word (1 = 32, 0 = 16).
 * @param type      The type of gate (trap or interrupt).
 **/
void idt_install_gate(uint16_t index, uint32_t segsel, uint32_t offset, 
		uint8_t p, uint8_t dpl, uint8_t d, uint8_t type);

/**
 * @brief Install a new gate in the IDT that will run at the given
 * priveledge level.
 *
 * The segment will be the kernel code segment, the privelege level
 * will be 3, D will be 1, and the gate will be marked as present.
 *
 * @param index The index into the idt to add the new gate.
 * @param handler The address of the handler.
 * @param type The type of gate (trap or interrupt).
 * @param dpl priveledge level (0,1,2,3).
 */
void idt_install_default_gate(uint16_t index, void (*handler)(), 
		uint8_t type, uint8_t dpl);

void idt_exn_gate(int vector, void (*handler)(void));
void idt_irq_gate(int irq, void (*handler)(void));
void idt_syscall_gate(int vector, void (*handler)(void));

#endif /* __IDT_H__ */
