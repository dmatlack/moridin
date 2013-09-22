/**
 * @file x86/io.h
 *
 * @brief Utility functions for communicating with x86 io ports.
 *
 * @author David Matlack
 */
#include <stdint.h>

// write 1 byte the given port
void outb(uint16_t port, uint8_t  val);
// write 2 byte the given port
void outw(uint16_t port, uint16_t val);
// write 4 byte the given port
void outl(uint16_t port, uint32_t val);

// read 1 byte from the given port
uint8_t inb(uint16_t port);
// read 2 byte from the given port
uint16_t inw(uint16_t port);
// read 4 byte from the given port
uint32_t ind(uint16_t port);

// Delay 1/8 microsecond
void iodelay(void);
