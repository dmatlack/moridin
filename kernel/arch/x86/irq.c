/**
 * @file arch/x86/irq.c
 *
 * @brief x86-dependent IRQ code.
 */
#include <arch/x86/irq.h>
#include <arch/x86/idt.h>
#include <arch/x86/pic.h>
#include <arch/x86/io.h>

#include <kernel/atomic.h>
#include <kernel/irq.h>

#include <debug.h>
#include <assert.h>

int spurious_irqs[PIC_IRQ_COUNT];

struct machine_irq_interface x86_irq_interface = {
  .init = x86_init_irq,
  .generate_irq = x86_generate_irq,
  .acknowledge_irq = x86_acknowledge_irq,
  .enable_irqs = x86_enable_irqs,
  .disable_irqs = x86_disable_irqs
};

int x86_init_irq(struct machine_irq_info *info) {
  int i, ret;
  
  info->max_irqs = PIC_IRQ_COUNT;

  /*
   * Initialize the hardware to receive interrupts.
   */
  if (0 != (ret = pic_init())) {
    return ret;
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

  for (i = 0; i < PIC_IRQ_COUNT; i++) spurious_irqs[i] = 0;

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
    __int(IDT_PIC_SLAVE_OFFSET + (irq - 8));
  }
}

/**
 * @brief Return true if the given IRQ is spurious.
 *
 * Very useful explaination of Spurious IRQS:
 *  http://wiki.osdev.org/8259_PIC#Spurious_IRQs
 *
 * @warning This function should only be called from interrupt
 * conext!
 */
bool is_spurious_irq(int irq) {
  uint16_t isr;

  if (0 > irq || irq >= PIC_IRQ_COUNT) return false;

  isr = pic_get_isr();

  return !(isr & (1 << irq));
}

/**
 * @brief This is the second level handler for all IRQs (the first
 * being the assembly entry points which are what are actually 
 * installed in the IDT). The job of this function is to pass 
 * the irq up to the kernel to handle.
 */
void x86_handle_irq(int irq) {

  ASSERT_GREATEREQ(irq, 0);
  ASSERT_LESS(irq, PIC_IRQ_COUNT);

  /*
   * Check for spurious IRQs
   */
  if (is_spurious_irq(irq)) {
    atomic_add(&spurious_irqs[irq], 1);
    WARN("Spurious IRQ: %d (total %d)", irq, spurious_irqs[irq]);

    /*
     * If the spurious IRQ is from the slave PIC, we still need to send an
     * EOI to the master.
     */
    if (irq >= 8) outb(PIC_MASTER_CMD, PIC_EOI);
    return;
  }

  //FIXME remove me
  //  This line reads a character from the keyboard data port so that
  //  we can continue receiving keyboard interrupts.
  if (1 == irq) inb(0x60);

  /*
   * Pass the interrupt request up to the kernel
   */
  handle_irq(irq);
}

/**
 * @brief Acknowledge the irq by sending the correct message to the PIC.
 */
void x86_acknowledge_irq(int irq) {
  pic_eoi(irq);
}

void x86_enable_irqs(void) {
  __enable_interrupts();
}

void x86_disable_irqs(void) {
  __disable_interrupts();
}


