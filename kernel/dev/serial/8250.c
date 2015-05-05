/**
 * @file dev/serial/8250.c
 *
 * @brief Device driver for 8250 UART.
 */
#include <dev/serial.h>
#include <dev/serial/8250.h>
#include <arch/io.h>
#include <lib/stddef.h>
#include <kernel/spinlock.h>

static int i8250_init(struct serial_port *s);
static void i8250_putchar(struct serial_port *s, char c);

struct i8250_port {
	struct serial_port serial;
	struct spinlock lock;
	u16 base;
	int irq;
};

#define DECLARE_I8250_PORT(_base, _irq, _name) {			\
	.serial = {							\
		.init = i8250_init,					\
		.putchar = i8250_putchar,				\
		.name = _name,						\
	},								\
	.base = _base,							\
	.irq = _irq,							\
}

struct i8250_port i8250_ports[] = {
	DECLARE_I8250_PORT(0x3f8, 4, "COM1"),
	DECLARE_I8250_PORT(0x2f8, 3, "COM2"),
	DECLARE_I8250_PORT(0x3e8, 4, "COM3"),
	DECLARE_I8250_PORT(0x3e8, 3, "COM4"),
};


struct i8250_port *to_i8250(struct serial_port *ptr)
{
	return container_of(ptr, struct i8250_port, serial);
}

static int i8250_init(struct serial_port *s)
{
	struct i8250_port *p = to_i8250(s);

	/*
	 * Disable interrupts from serial controller
	 */
	outb(p->base + SERIAL_PORT_LINE_CTL, 0x00);
	outb(p->base + SERIAL_PORT_IRQ, 0x00);

	/*
	 * Set the baud rate
	 */
	outb(p->base + SERIAL_PORT_LINE_CTL, 1 << 7);
	outb(p->base + SERIAL_PORT_BAUD_LSB, 0x03);
	outb(p->base + SERIAL_PORT_BAUD_MSB, 0x00);

	/*
	 * Tell the serial controller to use 8-bit characters, 1 stop bit, and
	 * not parity
	 */
	outb(p->base + SERIAL_PORT_LINE_CTL, 0x00);
	outb(p->base + SERIAL_PORT_LINE_CTL, 0x03);

	/*
	 * FIFO transmission buffering
	 */
	outb(p->base + SERIAL_PORT_FIFO_CTL,
			(1 << 0) |          // FIFO enable
			(1 << 1) |          // Clear receiver FIFO
			(1 << 2) |          // Clear transmitter FIFO
			(1 << 6) | (1 << 7) // Receiver FIFO trigger level 14
	    );

	outb(p->base + SERIAL_PORT_IRQ, 0x0B);

	return 0;
}

int early_i8250_putchar(int c)
{
	struct i8250_port *p = i8250_ports + 0;

	while ((inb(p->base + SERIAL_PORT_LINE_STATUS) & 0x20) == 0);
	outb(p->base + SERIAL_PORT_DATA, c);

	return 0;
}

static void i8250_putchar(struct serial_port *s, char c)
{
	struct i8250_port *p = to_i8250(s);
	unsigned long flags;

	spin_lock_irq(&p->lock, &flags);

	while ((inb(p->base + SERIAL_PORT_LINE_STATUS) & 0x20) == 0);
	outb(p->base + SERIAL_PORT_DATA, c);

	spin_unlock_irq(&p->lock, flags);
}

void init_8250(void)
{
	register_serial_port(&i8250_ports[0].serial);
	register_serial_port(&i8250_ports[1].serial);
	register_serial_port(&i8250_ports[2].serial);
	register_serial_port(&i8250_ports[3].serial);
}

void early_init_8250(void)
{
	return;
}
