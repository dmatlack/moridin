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
#include <lib/list.h>

struct serial_port {
	int (*init)(struct serial_port *);
	void (*putchar)(struct serial_port *, char c);
	const char *purpose;
	const char *name;
	bool reserved;
	list_link(struct serial_port) list;
};

list_typedef(struct serial_port) serial_port_list_t;

void register_serial_port(struct serial_port *s);
struct serial_port *reserve_serial_port(const char *purpose);
void serial_putchar(struct serial_port *s, char c);

#endif /* !__DEV_SERIAL_H__ */
