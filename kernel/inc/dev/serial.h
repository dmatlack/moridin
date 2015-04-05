/**
 * @file dev/serial.h
 *
 * References:
 *    http://wiki.osdev.org/Serial_Ports
 *    http://www.sci.muni.cz/docs/pc/serport.txt
 *      Read this document for a full description of programming the serial
 *      port and its controller.
 */
#ifndef __DEV_SERIAL_H__
#define __DEV_SERIAL_H__

#include <types.h>

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

struct serial_port {
	unsigned port;
	unsigned irq;
	bool reserved;
	const char *purpose;
};

void serial_port_init(void);
struct serial_port *reserve_serial_port(const char *purpose);
void serial_putchar(struct serial_port *s, char c);

#endif /* !__DEV_SERIAL_H__ */
