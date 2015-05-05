#ifndef __DEV_SERIAL_8250_H__
#define __DEV_SERIAL_8250_H__

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

int early_i8250_putchar(int c);
void init_8250(void);
void early_init_8250(void);

#endif /* !__DEV_SERIAL_8250_H__ */
