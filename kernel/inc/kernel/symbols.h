#ifndef __KERNEL_SYMBOLS_H__
#define __KERNEL_SYMBOLS_H__

#include <lib/stdint.h>

/*
 *
 * if you change anything in this file (struct symbol, symbol table length,
 * etc.), make sure tools/inject_symbol_table matches!!!!!
 *
 */

#define SYMBOL_TABLE_LENGTH 1024

struct symbol {
	u64 address;
#define BSS_SECTION	0x0
#define TEXT_SECTION	0x1
#define RO_SECTION	0x2
#define DATA_SECTION	0x3
#define UNKNOWN_SECTION	0x4

	u8 section;

#define SYMBOL_NAME_LENGTH 255
	char name[SYMBOL_NAME_LENGTH + 1]; /* +1 for null terminator */
} __attribute__(( packed ));

extern struct symbol symbol_table[SYMBOL_TABLE_LENGTH];

struct symbol *resolve_symbol(unsigned long address);

#endif /* !__KERNEL_SYMBOLS_H__ */
