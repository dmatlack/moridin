/**
 * @file arch/x86/irq.c
 */
#include <arch/x86/irq.h>
#include <arch/x86/idt.h>
#include <arch/x86/pic.h>
#include <kernel/kprintf.h>
#include <debug.h>
#include <arch/x86/io.h>

void generate_irq(int irq) {
  if (irq < 8) {
    __int(IDT_PIC_MASTER_OFFSET + irq);
  }
  else if (irq < 16) {
    __int(IDT_PIC_SLAVE_OFFSET + irq);
  }
}

void handle_irq(int irq) {
  switch (irq) {
    case 0: //timer
      break;
    case 1: //keyboard
      kprintf("IRQ: %d\n", irq);
      while ((inb(0x64) & 0x1) != 1) iodelay(); // wait for the scancode...
      kprintf("  scancode: 0x%02x\n", inb(0x60));

      break;
    default:
      kprintf("IRQ: %d\n", irq);
      break;
  }
  pic_eoi(irq);
}
