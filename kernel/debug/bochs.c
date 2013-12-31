/**
 * @file bochs.c
 *
 * @brief Debug utility for the bochs machine emulator.
 *
 * @author David Matlack
 */
#include <debug/bochs.h>
#include <kernel/io.h>

void bochs_putchar(char c) {
  outb(BOCHS_PUTCHAR_PORT, (uint8_t) c); 
}
