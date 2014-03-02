/**
 * @file arch/irq.c
 *
 * @brief x86-dependent IRQ code.
 */
#include <arch/irq.h>
#include <arch/idt.h>
#include <arch/pic.h>
#include <arch/io.h>
#include <arch/atomic.h>

#include <kernel/irq.h>

#include <kernel/debug.h>
#include <assert.h>

/*
 * Invoke the assembly instruction "int $n"
 */
void __int(uint8_t n);

/*
 * Entrypoints for IRQs. See irq_wrappers.S
 */
void irq_0(void);
void irq_1(void);
void irq_2(void);
void irq_3(void);
void irq_4(void);
void irq_5(void);
void irq_6(void);
void irq_7(void);
void irq_8(void);
void irq_9(void);
void irq_10(void);
void irq_11(void);
void irq_12(void);
void irq_13(void);
void irq_14(void);
void irq_15(void);

int spurious_irqs[MAX_NUM_IRQS];

void pic_irq_init(void) {
  int i;
  
  /*
   * Install the IRQ handlers
   */
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 0, irq_0);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 1, irq_1);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 2, irq_2);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 3, irq_3);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 4, irq_4);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 5, irq_5);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 6, irq_6);
  idt_irq_gate(IDT_PIC_MASTER_OFFSET + 7, irq_7);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 0, irq_8);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 1, irq_9);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 2, irq_10);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 3, irq_11);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 4, irq_12);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 5, irq_13);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 6, irq_14);
  idt_irq_gate(IDT_PIC_SLAVE_OFFSET  + 7, irq_15);

  /*
   * Remap the master and the slave so they invode the correct service routines
   * in the IDT.
   */
  pic_remap(IDT_PIC_MASTER_OFFSET, IDT_PIC_SLAVE_OFFSET);

  for (i = 0; i < MAX_NUM_IRQS; i++) spurious_irqs[i] = 0;
}

/**
 * @brief Generate an interrupt request.
 */
void generate_irq(int irq) {
  ASSERT_GREATEREQ(irq, 0);
  ASSERT_LESS(irq, 16);

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

  if (0 > irq || irq >= MAX_NUM_IRQS) return false;

  isr = pic_get_isr();

  return !(isr & (1 << irq));
}

/**
 * @brief This is the second level handler for all IRQs (the first
 * being the assembly entry points which are what are actually 
 * installed in the IDT). The job of this function is to pass 
 * the irq up to the kernel to handle.
 */
void interrupt_request(int irq) {

  ASSERT_GREATEREQ(irq, 0);
  ASSERT_LESS(irq, MAX_NUM_IRQS);

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

  //TODO remove me
  //  This line reads a character from the keyboard data port so that
  //  we can continue receiving keyboard interrupts.
  if (1 == irq) inb(0x60);

  /*
   * Pass the interrupt request up to the kernel
   */
  kernel_irq_handler(irq);
}

/**
 * @brief Acknowledge the irq by sending the correct message to the PIC.
 */
void x86_acknowledge_irq(int irq) {
  pic_eoi(irq);
}
