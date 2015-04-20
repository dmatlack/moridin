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
#include <kernel/spinlock.h>

serial_port_list_t serial_ports = INITIALIZED_EMPTY_LIST;
struct spinlock serial_ports_lock = INITIALIZED_SPINLOCK;

void register_serial_port(struct serial_port *s)
{
	unsigned long flags;
	int ret;

	ret = s->init(s);
	if (ret)
		return;

	spin_lock_irq(&serial_ports_lock, &flags);
	list_insert_tail(&serial_ports, s, list);
	spin_unlock_irq(&serial_ports_lock, flags);
}

struct serial_port *reserve_serial_port(const char *purpose)
{
	struct serial_port *reserved = NULL;
	struct serial_port *s;
	unsigned long flags;

	spin_lock_irq(&serial_ports_lock, &flags);

	list_foreach(s, &serial_ports, list) {
		if (!s->reserved) {
			s->purpose = purpose;
			s->reserved = true;
			reserved = s;
			break;
		}
	}

	spin_unlock_irq(&serial_ports_lock, flags);

	return reserved;
}

void serial_putchar(struct serial_port *s, char c)
{
	if (s->putchar)
		s->putchar(s, c);
}
