/**
 * @file debug.c
 */
#include <debug.h>
#include <kernel.h>
#include <dev/serial.h>
#include <debug/bochs.h>

struct serial_port *debug_serial_port;

int debug_putchar(int c) {
  if (debug_serial_port) {
    serial_putchar(debug_serial_port, (char) c);
  }
#ifdef BOCHS
  bochs_putchar(c);
#endif
  return c;
}

int debug_init(void) {

  debug_serial_port = reserve_serial_port("debug");
  if (!debug_serial_port) {
    kprintf("Could not reserve serial port for debugging.\n");
  }

  return 0;
}
