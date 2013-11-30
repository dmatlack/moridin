/**
 * @file arch/x86/irq.c
 */
#include <arch/x86/irq.h>
#include <arch/x86/idt.h>

void generate_irq(uint8_t irq) {
  if (irq < 8) {
    __int(IDT_PIC_MASTER_OFFSET + irq);
  }
  else if (irq < 16) {
    __int(IDT_PIC_SLAVE_OFFSET + irq);
  }
}
