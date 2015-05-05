/**
 * @file debug/debug.c
 */
#include <kernel/debug.h>
#include <kernel/config.h>
#include <kernel/log.h>
#include <dev/bochs.h>

#include <dev/serial.h>

struct serial_port *debug_serial_port;

int debug_putchar(int c)
{
	if (debug_serial_port) {
		serial_putchar(debug_serial_port, (char) c);
	}
#ifdef BOCHS
	bochs_putchar(c);
#endif
	return c;
}

void debug_init(void)
{
	debug_serial_port = reserve_serial_port("debug");
	if (!debug_serial_port) {
		panic("Could not reserve serial port for debugging.\n");
	}
	log_init(debug_putchar, CONFIG_LOG_LEVEL);
}
