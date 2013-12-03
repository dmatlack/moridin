/**
 * @file arch/x86/irq.c
 *
 * @brief x86-dependent IRQ code.
 */
#include <arch/x86/irq.h>
#include <arch/x86/idt.h>
#include <arch/x86/pic.h>
#include <arch/x86/io.h>

#include <kernel/irq.h>

#include <debug.h>
#include <assert.h>

struct machine_irq_interface x86_irq_interface = {
  .init = x86_init_irq,
  .generate_irq = x86_generate_irq,
  .acknowledge_irq = x86_acknowledge_irq
};

int x86_init_irq(struct machine_irq_info *info) {
  
  info->max_irqs = PIC_IRQ_COUNT;

  /*
   * Initialize the hardware to receive interrupts.
   */
  if (0 != pic_init()) {
    return -1;
  }

  /*
   * Install the IRQ handlers
   */
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 0, __irq0);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 1, __irq1);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 2, __irq2);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 3, __irq3);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 4, __irq4);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 5, __irq5);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 6, __irq6);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 7, __irq7);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 0, __irq8);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 1, __irq9);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 2, __irq10);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 3, __irq11);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 4, __irq12);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 5, __irq13);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 6, __irq14);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 7, __irq15);

  return 0;
}

/**
 * @brief Generate an interrupt request.
 */
void x86_generate_irq(int irq) {
  if (irq < 8) {
    __int(IDT_PIC_MASTER_OFFSET + irq);
  }
  else if (irq < 16) {
    __int(IDT_PIC_SLAVE_OFFSET + irq);
  }
}

/**
 * @brief This is the second level handler for all IRQs (the first
 * being the assembly entry points which are what are actually 
 * installed in the IDT). The job of this function is to pass 
 * the irq up to the kernel to handle.
 */
void x86_handle_irq(int irq) {
  ASSERT(irq >= 0 && irq < PIC_IRQ_COUNT);

  if (irq == 1) inb(0x60); //FIXME remove me

  /*
   * pass the interrupt request up to the kernel
   */
  handle_irq(irq);
}

/**
 * @brief Acknowledge the irq by sending the correct message to the PIC.
 */
void x86_acknowledge_irq(int irq) {
  pic_eoi(irq);
}
