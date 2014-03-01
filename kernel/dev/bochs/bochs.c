/**
 * @file bochs.c
 *
 * @brief Debug utility for the bochs machine emulator.
 *
 */
#include <dev/bochs.h>
#include <arch/io.h>

void bochs_putchar(char c) {
  outb(BOCHS_PUTCHAR_PORT, (uint8_t) c); 
}
