/**
 * @file bochs.h
 *
 * @brief Debug utility for the bochs machine emulator.
 *
 * @author David Matlack
 */
#ifndef __BOCHS_H__
#define __BOCHS_H__

#define BOCHS_PUTCHAR_PORT 0xE9 // compile bochs with the e9 hack!

void bochs_putchar(char c);

#endif /* __BOCHS_H__ */
