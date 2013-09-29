/**
 * @file pic.c
 *
 * @author David Matlack
 */
#include <x86/pic.h>
#include <x86/io.h>

int pic_init(uint32_t master_offset, uint32_t slave_offset) {
  pic_remap(master_offset, slave_offset);
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
