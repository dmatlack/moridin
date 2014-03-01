/**
 * @file bochs.h
 *
 * @brief Debug utility for the bochs machine emulator.
 *
 */
#ifndef __BOCHS_H__
#define __BOCHS_H__

#define BOCHS

#define BOCHS_PUTCHAR_PORT 0xE9 // compile bochs with the e9 hack!

#define BOCHS_MAGIC_BREAK __asm__("xchg %bx, %bx")

void bochs_putchar(char c);

#endif /* __BOCHS_H__ */
