/**
 * @file dev/pit.h
 *
 * Programmable Interrupt Timer (PIT), 8253/8254 Chip
 *
 * @author David Matlack
 */
#ifndef __DEV_PIT_H__
#define __DEV_PIT_H__

#include <arch/x86/pic.h>

extern struct pic_device timer_device;

int pit_init(void (*handler)(void));

#endif /* !__DEV_PIT_H__ */
