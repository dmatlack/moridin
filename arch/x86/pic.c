/**
 * @file pic.c
 *
 * @author David Matlack
 */
#include <arch/x86/pic.h>
#include <arch/x86/io.h>
#include <arch/x86/idt.h>
#include <debug.h>
#include <kernel/kprintf.h>

int pic_init(uint32_t master_offset, uint32_t slave_offset) {
  TRACE("master_offset=0x%08x, slave_offset=0x%08x",
        master_offset, slave_offset);

  /*
   * Remap the master and the slave so they invode the correct service routines
   * in the IDT.
   */
  pic_remap(master_offset, slave_offset);

  /*
   * Install the IRQ handlers
   */
  idt_irq_gate(master_offset + 0, __irq0);
  idt_irq_gate(master_offset + 1, __irq1);
  idt_irq_gate(master_offset + 2, __irq2);
  idt_irq_gate(master_offset + 3, __irq3);
  idt_irq_gate(master_offset + 4, __irq4);
  idt_irq_gate(master_offset + 5, __irq5);
  idt_irq_gate(master_offset + 6, __irq6);
  idt_irq_gate(master_offset + 7, __irq7);
  idt_irq_gate(slave_offset  + 0, __irq8);
  idt_irq_gate(slave_offset  + 1, __irq9);
  idt_irq_gate(slave_offset  + 2, __irq10);
  idt_irq_gate(slave_offset  + 3, __irq11);
  idt_irq_gate(slave_offset  + 4, __irq12);
  idt_irq_gate(slave_offset  + 5, __irq13);
  idt_irq_gate(slave_offset  + 6, __irq14);
  idt_irq_gate(slave_offset  + 7, __irq15);

  return 0;
}

void pic_remap(uint32_t master_offset, uint32_t slave_offset) {
  uint8_t slave_irq_mask, master_irq_mask;

  // save the current interrupt mask
  master_irq_mask = inb(PIC_MASTER_DATA);
  slave_irq_mask = inb(PIC_SLAVE_DATA);

  // tell the pics that there are 3 data messages coming!
  outb(PIC_MASTER_CMD, PIC_INIT);
  iodelay();
  outb(PIC_SLAVE_CMD, PIC_INIT);
  iodelay();

  // icw2: set the vector offset
  outb(PIC_MASTER_DATA, master_offset);
  iodelay();
  outb(PIC_SLAVE_DATA, slave_offset);
  iodelay();

  // icw3: master/slaving wiring
  outb(PIC_MASTER_DATA, PIC_ICW3_TELL_MASTER_ABOUT_SLAVE);
  iodelay();
  outb(PIC_SLAVE_DATA, PIC_ICW3_TELL_SLAVE_CASCADE_ID);
  iodelay();

  // icw4: describe environment
  outb(PIC_MASTER_DATA, PIC_ICW4_8086);
  iodelay();
  outb(PIC_SLAVE_DATA, PIC_ICW4_8086);
  iodelay();

  // restore the masks
  outb(PIC_MASTER_DATA, master_irq_mask);
  outb(PIC_SLAVE_DATA, slave_irq_mask);
}

void pic_imr_set(uint8_t irq) {
  uint32_t port;
  uint8_t mask;

  if (irq < 8) {
    port = PIC_MASTER_DATA;
  }
  else {
    port = PIC_SLAVE_DATA;
    irq -= 8;
  }

  mask = inb(port) | (1 << irq);
  outb(port, mask);
}

void pic_imr_clear(uint8_t irq) {
  uint32_t port;
  uint8_t mask;

  if (irq < 8) {
    port = PIC_MASTER_DATA;
  }
  else {
    port = PIC_SLAVE_DATA;
    irq -= 8;
  }

  mask = inb(port) & ~(1 << irq);
  outb(port, mask);
}

static inline uint16_t pic_get_reg(uint8_t ocw3) {
  outb(PIC_MASTER_CMD, ocw3);
  outb(PIC_SLAVE_CMD, ocw3);

  return inb(PIC_MASTER_CMD) | (inb(PIC_SLAVE_CMD) << 8);
}

uint16_t pic_get_isr(void) {
  return pic_get_reg(PIC_READ_ISR);
}

uint16_t pic_get_irr(void) {
  return pic_get_reg(PIC_READ_IRR);
}

void pic_eoi(uint8_t irq) {
  if (irq >= 8) {
    outb(PIC_SLAVE_CMD, PIC_EOI);
  }
  outb(PIC_MASTER_CMD, PIC_EOI);
}
