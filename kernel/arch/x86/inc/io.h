/**
 * @file x86/io.h
 *
 * @brief Utility functions for communicating with x86 io ports.
 */
#include <stdint.h>

/** 
 * @brief write 1 byte the given port 
 */
void outb(u16 port, u8 val);
/** 
 * @brief write 2 byte the given port 
 */
void outw(u16 port, u16 val);
/** 
 * @brief write 4 byte the given port 
 */
void outl(u16 port, u32 val);

/**
 * @brief read 1 byte from the given port 
 */
u8 inb(u16 port);
/** 
 * @brief read 2 byte from the given port 
 */
u16 inw(u16 port);
/** 
 * @brief read 4 byte from the given port 
 */
u32 inl(u16 port);

// Delay 1/8 microsecond
void iodelay(void);
