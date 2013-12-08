/**
 * @file x86/io.h
 *
 * @brief Utility functions for communicating with x86 io ports.
 */
#include <stdint.h>

/** 
 * @brief write 1 byte the given port 
 */
void outb(uint16_t port, uint8_t  val);
/** 
 * @brief write 2 byte the given port 
 */
void outw(uint16_t port, uint16_t val);
/** 
 * @brief write 4 byte the given port 
 */
void outd(uint16_t port, uint32_t val);

/**
 * @brief read 1 byte from the given port 
 */
uint8_t inb(uint16_t port);
/** 
 * @brief read 2 byte from the given port 
 */
uint16_t inw(uint16_t port);
/** 
 * @brief read 4 byte from the given port 
 */
uint32_t ind(uint16_t port);

// Delay 1/8 microsecond
void iodelay(void);
