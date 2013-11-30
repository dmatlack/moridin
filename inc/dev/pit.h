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

#define PIT_FREQ_HZ 1193182

#define PIT_CHANNEL0_PORT    0x40   // IRQ 0
#define PIT_CHANNEL1_PORT    0x41   // obsolete
#define PIT_CHANNEL2_PORT    0x42   // pc speaker
#define PIT_COMMAND_PORT     0x43

extern struct pic_device timer_device;

/*
 * PIT Command Register format
 */
#define IRQ_CHANNEL       (0 << 6)
#define CHANNEL1          (1 << 6)
#define SPEAKER_CHANNEL   (2 << 6)
#define READBACK          (3 << 6)

#define LATCH_COUNT       (0 << 4)
#define LOBYTE            (1 << 4)
#define HIBYTE            (2 << 4)
#define LOHIBYTE          (3 << 4)

#define OPMODE0           (0 << 2) // interrupt on terminal count
#define OPMODE1           (1 << 2) // hardware re-triggerable one-shot
#define OPMODE2           (2 << 2) // rate generator
#define SQUARE_WAVE       (3 << 2) // square wave generator
#define OPMODE4           (4 << 2) // software triggered strobe
#define OPMODE5           (5 << 2) // hardware triggered strobe
#define OPMODE6           (6 << 2) // same as (2)
#define OPMODE7           (7 << 2) // same as (3)

#define BINARYMODE  (0 << 0) // 0  6-bit binary
#define BCDMODE     (1 << 0) // 1  our-digit BCD

int pit_init(int hz, void (*handler)(void));

#endif /* !__DEV_PIT_H__ */
