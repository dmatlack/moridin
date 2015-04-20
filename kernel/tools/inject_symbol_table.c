/**
 * @file tools/inject_symbol_table.c
 *
 * Example:
 *
 * $ ./tools/inject_symbol_table kernel/symbols.o 0x0000234
 */
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t

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
	char name[SYMBOL_NAME_LENGTH + 1];
} __attribute__(( packed ));

#define CHECK(_condition) do {						\
	if (_condition) break;						\
									\
	fprintf(stderr, "CHECK FAILED: %s\n", #_condition);		\
	perror("");							\
	exit(2);							\
} while (0);

struct symbol symbol_table[SYMBOL_TABLE_LENGTH];

void usage(void)
{
	fprintf(stderr,
	"USAGE: nm -n object-file | inject_symbol_table <file> <offset>\n"
	"\n"
	"Inject a binary symbol table into <file> at offset <offset>. This\n"
	"binary expects the output of nm to be fed into stdin. The output of\n"
	"nm will be converted into the binary symbol table.\n"
	);
	exit(1);
}

static int to_section(char c)
{
	switch (c) {
		case 't':
		case 'T': return TEXT_SECTION;

		case 'd':
		case 'D': return DATA_SECTION;

		case 'b':
		case 'B': return BSS_SECTION;

		case 'r':
		case 'R': return RO_SECTION;
	}

	return UNKNOWN_SECTION;
}

static void line_to_symbol(char *line, struct symbol *symbol)
{
	char *s;

	memset(symbol, 0, sizeof(*symbol));

	/* example line: 001127d9 T __xchg */

	/* address */
	s = strtok(line, " \n");
	symbol->address = strtoul(s, NULL, 16);
	CHECK(symbol->address);

	/* section */
	s = strtok(NULL, " \n");
	CHECK(strlen(s) == 1);
	symbol->section = to_section(*s);

	/* name */
	s = strtok(NULL, " \n");
	CHECK(strlen(s) <= SYMBOL_NAME_LENGTH);
	strcpy(symbol->name, s);

	if (symbol > symbol_table)
		/* symbols must be sorted by address */
		CHECK(symbol->address >= (symbol - 1)->address);
}

int main(int argc, char **argv)
{
	static const unsigned bufsz = 1024;
	char buf[bufsz];
	char *filename;
	unsigned long offset;
	struct stat stat;
	void *mapping;
	int index = 0;
	int fd;

	if (argc != 3)
		usage();

	filename = argv[1];
	offset = strtoul(argv[2], NULL, 0);
	CHECK(offset);

	while (fgets(buf, bufsz, stdin)) {
		/*
		 * simplify our life: just fail if bufsz
		 * isn't big enough for one line. if this
		 * fails just increase bufsz.
		 */
		CHECK(buf[strlen(buf) - 1] == '\n');

		/*
		 * make sure the symbol table is large enough.
		 * if this check fails SYMBOL_TABLE_LENGTH needs
		 * to be increased and the kernel re-compiled.
		 *
		 * we reserve 1 extra symbol table entry for a
		 * null, marking the end of the table.
		 */
		CHECK(index < (SYMBOL_TABLE_LENGTH - 1));

		line_to_symbol(buf, symbol_table + index);

		index++;
	}

	/* zero the final symbol table entry */
	memset(symbol_table + index, 0, sizeof(struct symbol));

	fd = open(filename, O_RDWR);
	CHECK(fd != -1);

	mapping = mmap(NULL, sizeof(symbol_table) + offset,
		       PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	CHECK(mapping != MAP_FAILED);

	/*
	 * NOT PORTABLE!!!!!
	 *
	 * If the host platform endianess does not match the target
	 * (kernel) platform endianess, this will not work.
	 */
	memcpy(mapping + offset, symbol_table, sizeof(symbol_table));

	munmap(mapping, sizeof(symbol_table) + offset);
	CHECK(!close(fd));

	return 0;
}
