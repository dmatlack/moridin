/**
 * @file idtr.h
 *
 * @breif Function for manipulating the Interrupt Descriptor Table 
 * Register (IDTR).
 *
 *       .------------+-----------------------.
 * IDTR: |0  LIMIT  15|16      BASE         47|
 *       '------------+-----------------------'
 *
 *    Limit: Defines the length of the IDT in bytes. The minimum value 
 *           is 0x100 and the maximum is 0x1000 (which corresponds to 
 *           0x200 interrupts).
 *    Base:  The 32-bit physical address where the IDT starts (INT 0).
 *
 */
#ifndef __IDTR_H__
#define __IDTR_H__

/**
 * @brief Return the 32-bit base address of the Interrupt Descriptor Table.
 */
uint32_t idt_get_base(void); //TODO test me

/**
 * @brief Return the 16-bit limit (size) of the Interrupt Descriptor Table.
 */
uint16_t idt_get_limit(void); //TODO test me

#endif /* __IDTR_H_ */
