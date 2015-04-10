/**
 * @file dev/serial/8250.c
 *
 * @brief Device driver for 8250 UART.
 */
#include <dev/serial.h>
#include <arch/io.h>
#include <lib/stddef.h>

/*
 * Offsets into the Serial Port I/O Space.
 *
 * The first 2 bytes can be used in 2 ways. When DLAB (Divisor Latch Access
 * Bit, bit 7 of SERIAL_PORT_LINE_CTL) is set, the first two bytes act
 * as writable registers for setting the Baud divisor. When DLAB is not set,
 * the first two bytes act as the DATA and IRQ registers.
 */
#define SERIAL_PORT_DATA          0x00
#define SERIAL_PORT_IRQ           0x01
#define SERIAL_PORT_BAUD_LSB      0x00
#define SERIAL_PORT_BAUD_MSB      0x01
#define SERIAL_PORT_FIFO_CTL      0x02
#define SERIAL_PORT_IIR           0x02
#define SERIAL_PORT_LINE_CTL      0x03
#define SERIAL_PORT_MODEM_CTL     0x04
#define SERIAL_PORT_LINE_STATUS   0x05
#define SERIAL_PORT_MODEM_STATUS  0x06
#define SERIAL_PORT_SRATCH        0x07

struct i8250_port {
	struct serial_port serial;
	u16 base;
	int irq;
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

static void i8250_putchar(struct serial_port *s, char c)
{
	struct i8250_port *p = to_i8250(s);

	while ((inb(p->base + SERIAL_PORT_LINE_STATUS) & 0x20) == 0);
	outb(p->base + SERIAL_PORT_DATA, c);
}

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

void init_8250(void)
{
	register_serial_port(&i8250_ports[0].serial);
	register_serial_port(&i8250_ports[1].serial);
	register_serial_port(&i8250_ports[2].serial);
	register_serial_port(&i8250_ports[3].serial);
}
