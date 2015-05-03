/**
 * @file pic.c
 *
 */
#include <arch/pic.h>
#include <arch/io.h>
#include <arch/idt.h>
#include <kernel/debug.h>

void pic_remap(u32 master_offset, u32 slave_offset)
{
	u8 slave_irq_mask, master_irq_mask;

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

void pic_imr_set(u8 irq)
{
	u32 port;
	u8 mask;

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

void pic_imr_clear(u8 irq)
{
	u32 port;
	u8 mask;

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

static inline u16 pic_get_reg(u8 ocw3)
{
	u8 master, slave;

	outb(PIC_MASTER_CMD, ocw3);
	master = inb(PIC_MASTER_CMD);

	outb(PIC_SLAVE_CMD, ocw3);
	slave = inb(PIC_SLAVE_CMD);

	return (slave << 8) | (master);
}

u16 pic_get_isr(void)
{
	return pic_get_reg(PIC_READ_ISR);
}

u16 pic_get_irr(void)
{
	return pic_get_reg(PIC_READ_IRR);
}

void pic_eoi(u8 irq)
{
	if (irq >= 8) {
		outb(PIC_SLAVE_CMD, PIC_EOI);
	}
	outb(PIC_MASTER_CMD, PIC_EOI);
}
