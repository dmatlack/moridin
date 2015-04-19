#include <kernel/compiler.h>
#include <kernel/symbols.h>
#include <lib/stdint.h>
#include <lib/stddef.h>

struct symbol symbol_table[SYMBOL_TABLE_LENGTH] __section(".symbols");

struct symbol *resolve_symbol(unsigned long address)
{
	struct symbol *s = symbol_table;
	struct symbol *prev = NULL;
	struct symbol *sym = NULL;

	while (s->name[0]) {
		/* symbol table is sorted by address, low to high. */
		if (address < s->address) {
			sym = prev;
			break;
		}

		prev = s++;
	}

	return sym;
}
