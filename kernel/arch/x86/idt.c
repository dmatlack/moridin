/**
 * @file idt.c
 *
 * @brief Functions for adding entries to the Interrupt Descriptor Table.
 *
 **/
#include <arch/idt.h>
#include <arch/idtr.h>
#include <arch/seg.h>

/* Used for creating the IDT gate descriptors */
#define LS_2_BYTES 0x0000FFFF
#define MS_2_BYTES 0xFFFF0000
#define SEGSEL_SHIFT   16
#define P_MASK         1
#define P_SHIFT        15
#define DPL_MASK       3 // 11 (base 2)
#define DPL_SHIFT      13
#define D_MASK         1
#define D_SHIFT        2
#define D_CONSTANT     3 // 11 (base 2)
#define D_SHIFT_SHIFT  9
#define TYPE_MASK      1
#define TYPE_SHIFT     8
#define GATE_SIZE      64

/** 
 * @brief Return the "upper" portion of the gate descriptor.
 *
 *  -----------------------------------------------------------
 *  | Offset [31...16] | P |  DPL  | 0D11 | Type | 000 | ---- |
 *  -----------------------------------------------------------
 *  |31              16|15 |14   13|12   9|  8   |7   5|4    0| <- bit indices 
 * 
 * Offset[31...16]   Bits 16-31 (the upper 16 bits) of the offset into the code
 *                   segment where the interrupt handler is located.
 *
 * P                 Segment Present Flag
 *
 * DPL               Descriptor Privelege Level
 *
 * D                 Size of Gate (1 = 32 btis, 0 = 16 bits)
 *
 * Type              The type of gate (trap or interrupt).
 *
 * @param g A pointer to a gate_info_t struct.
 *
 * @return The 32-bit word described above.
 **/
static inline uint32_t gd_upper(idt_gate_t *gate) {
  uint32_t offset_mask = (MS_2_BYTES & gate->offset);
  uint32_t p_mask      = ((P_MASK & gate->p) << P_SHIFT);
  uint32_t dpl_mask    = ((DPL_MASK & gate->dpl) << DPL_SHIFT);
  uint32_t d_mask      = ((((D_MASK & gate->d) << D_SHIFT) + D_CONSTANT ) 
                          << D_SHIFT_SHIFT);
  uint32_t type_mask   = ((TYPE_MASK & gate->type) << TYPE_SHIFT);
  
  return (0xFFFFFF00) & (offset_mask | p_mask | dpl_mask | d_mask | type_mask);
}

/** 
 * @brief Return the "lower" portion of the gate descriptor.
 *
 *  --------------------------------------
 *  | Segment Selector | Offset [15...0] |
 *  --------------------------------------
 *  |31              16|15              0| <- bit indices
 * 
 * Segment Selector  Address of code segment where out handler is located.
 *
 * Offset[31...16]   Bits 0-15 (the lower 16 bits) of the offset into the code
 *                   segment where the interrupt handler is located.
 *
 * @param segsel  The Segment Selector
 * @param offset  The (full) Offset
 *
 * @return The 32-bit word described above.
 **/
static inline uint32_t gd_lower(idt_gate_t *gate) {
  uint32_t segsel_mask = ((LS_2_BYTES & gate->segsel) << SEGSEL_SHIFT);
  uint32_t offset_mask =  (LS_2_BYTES & gate->offset);

  return (segsel_mask | offset_mask);  
}

void idt_install_gate(uint16_t index, uint32_t segsel, uint32_t offset, 
                      uint8_t p, uint8_t dpl, uint8_t d, uint8_t type) {
  uint32_t base = (uint32_t) idt_get_base();
  idt_gate_t gate;

  // Build the gate descriptor struct
  gate.segsel = segsel;
  gate.offset = offset;
  gate.p = p;
  gate.dpl = dpl; 
  gate.d = d;
  gate.type = type;

  // write the gate descriptor to the IDT
  *(uint32_t *)(base + index * IDT_GATE_SIZE) = gd_lower(&gate);
  *(uint32_t *)(base + index * IDT_GATE_SIZE + IDT_GATE_SIZE/2) = gd_upper(&gate);
}

void idt_install_default_gate(uint16_t index, void (*handler)(), uint8_t type,
                              uint8_t dpl) {
  idt_install_gate(index, SEGSEL_KERNEL_CS, (uint32_t) handler, 
                   IDT_GATE_PRESENT, dpl, IDT_D_32, type);
}

void idt_exn_gate(int vector, void (*handler)()) {
  idt_install_gate(
      (uint16_t) IDT_EXN_OFFSET + vector,
      SEGSEL_KERNEL_CS,
      (uint32_t) handler,
      IDT_GATE_PRESENT,
      IDT_PL3,
      IDT_D_32,
      IDT_GATE_TYPE_TRAP);
}

void idt_syscall_gate(int vector, void (*handler)(void)) {
  idt_install_gate(
      (uint16_t) vector,
      SEGSEL_KERNEL_CS,
      (uint32_t) handler,
      IDT_GATE_PRESENT,
      IDT_PL3,
      IDT_D_32,
      IDT_GATE_TYPE_TRAP);
}

void idt_irq_gate(int irq, void (*handler)()) {
  idt_install_gate(
      irq,
      SEGSEL_KERNEL_CS,
      (uint32_t) handler,
      IDT_GATE_PRESENT,
      IDT_PL0,
      IDT_D_32,
      IDT_GATE_TYPE_INT);
}
