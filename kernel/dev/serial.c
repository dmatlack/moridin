/**
 * @file dev/serial.c
 *
 * @brief Device driver for the serial port.
 */
#include <dev/serial.h>

#include <stddef.h>
#include <types.h>

#include <arch/io.h>

//static unsigned default_serial_ports[4] = { 0x3F8, 0x2F8, 0x3E8, 0x2E8 };
static unsigned default_serial_irqs[4]  = {     4,     3,     4,     3 };

struct serial_port __serial_ports[4];

static void init_port(struct serial_port *s)
{
	/*
	 * Disable interrupts from serial controller
	 */
	outb(s->port + SERIAL_PORT_LINE_CTL, 0x00);
	outb(s->port + SERIAL_PORT_IRQ, 0x00);

	/*
	 * Set the baud rate
	 */
	outb(s->port + SERIAL_PORT_LINE_CTL, 1 << 7);
	outb(s->port + SERIAL_PORT_BAUD_LSB, 0x03);
	outb(s->port + SERIAL_PORT_BAUD_MSB, 0x00);

	/*
	 * Tell the serial controller to use 8-bit characters, 1 stop bit, and
	 * not parity
	 */
	outb(s->port + SERIAL_PORT_LINE_CTL, 0x00);
	outb(s->port + SERIAL_PORT_LINE_CTL, 0x03);

	/*
	 * FIFO transmission buffering
	 */
	outb(s->port + SERIAL_PORT_FIFO_CTL,
			(1 << 0) |          // FIFO enable
			(1 << 1) |          // Clear receiver FIFO
			(1 << 2) |          // Clear transmitter FIFO
			(1 << 6) | (1 << 7) // Receiver FIFO trigger level 14
	    );

	outb(s->port + SERIAL_PORT_IRQ, 0x0B);
}

/**
 * @brief Find and initialize all serial ports connected to the computer.
 */
void serial_port_init(void)
{
	struct serial_port *s;
	unsigned i;

	for (i = 0; i < arraylen(__serial_ports); i++) {
		s = &__serial_ports[i];

		s->port = *((uint16_t *) (0x400 + 2*i));
		s->irq = default_serial_irqs[i];
		s->purpose = NULL;
		s->reserved = false;

		if (s->port)
			init_port(s);
	}
}

struct serial_port *reserve_serial_port(const char *purpose)
{
	struct serial_port *s;
	unsigned i;

	for (i = 0; i < arraylen(__serial_ports); i++) {
		s = __serial_ports + i;

		if (!s->reserved && 0 != s->port) {
			s->purpose = purpose;
			s->reserved = true;
			return s;
		}
	}

	return NULL;
}

void serial_putchar(struct serial_port *s, char c)
{
	while ((inb(s->port + SERIAL_PORT_LINE_STATUS) & 0x20) == 0);
	outb(s->port + SERIAL_PORT_DATA, c);
}
