/**
 * @file dev/serial/serial.c
 *
 * @brief Serial subsystem.
 */
#include <arch/io.h>
#include <dev/serial.h>
#include <lib/list.h>
#include <stddef.h>
#include <types.h>

serial_port_list_t serial_ports = INITIALIZED_EMPTY_LIST;

void register_serial_port(struct serial_port *s)
{
	int ret;

	ret = s->init(s);
	if (ret)
		return;

	list_insert_tail(&serial_ports, s, list);
}

struct serial_port *reserve_serial_port(const char *purpose)
{
	struct serial_port *s;

	list_foreach(s, &serial_ports, list) {
		if (!s->reserved) {
			s->purpose = purpose;
			s->reserved = true;
			return s;
		}
	}

	return NULL;
}

void serial_putchar(struct serial_port *s, char c)
{
	if (s->putchar)
		s->putchar(s, c);
}
