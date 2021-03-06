/**
 * @file lib/elf/elf32.h
 *
 * @brief ELF (Executable and Linking Format) for 32-bit operating systems.
 *
 * References:
 *  http://www.skyfree.org/linux/references/ELF_Format.pdf
 *
 */
#ifndef __LIB_ELF_ELF32_H__
#define __LIB_ELF_ELF32_H__

#include <stdint.h>

typedef uint32_t elf32_addr_t;
typedef uint16_t elf32_half_t;
typedef uint32_t elf32_off_t;
typedef int32_t  elf32_sword_t; // NOTE: elf uses "word" to denote a 4-byte
typedef uint32_t elf32_word_t;  // value, which is different from the kernel

#define ELFMAG0 ((char) 0x7f)
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

#define ELFDATANONE  0
#define ELFDATA2LSB  1 // little endian, two's complement
#define ELFDATA2MSB  2 // big endian, two's complement

struct elf32_ehdr {
#define EI_MAG0    0
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5
#define EI_VERSION 6
#define EI_PAD     7
#define EI_NIDENT  16
	unsigned char e_ident[EI_NIDENT];

#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff
	elf32_half_t e_type;

#define EM_NONE  0 // no machine
#define EM_M32   1 // AT&T WE 32100
#define EM_SPACE 2 // SPARC
#define EM_386   3 // Intel 80386
#define EM_68K   4 // Motorola 68000
#define EM_88K   5 // Motorola 88000
#define EM_860   7 // Intel 80860
#define EM_MIPS  8 // MIPS RS3000
	elf32_half_t e_machine;

#define EV_NONE    0 // invalid version
#define EV_CURRENT 1 // current version
	elf32_word_t e_version;

	elf32_addr_t e_entry;     // program entry point
	elf32_off_t  e_phoff;     // program headers
	elf32_off_t  e_shoff;     // section headers
	elf32_word_t e_flags;     // ignore
	elf32_half_t e_ehsize;    // header size in bytes
	elf32_half_t e_phentsize; // program header size in bytes
	elf32_half_t e_phnum;     // num program headers
	elf32_half_t e_shentsize; // section header size in bytes
	elf32_half_t e_shnum;     // num section headers
	elf32_half_t e_shstrndx;  // index of string table section header
} __attribute__((packed));

#define CASE(_macro) case (_macro): return #_macro
static inline const char *elf32_type(elf32_half_t type) {
	switch (type) {
		CASE(ET_NONE);
		CASE(ET_REL);
		CASE(ET_EXEC);
		CASE(ET_DYN);
		CASE(ET_CORE);
	default: return "?";
	}
}
static inline const char *elf32_machine(elf32_half_t machine) {
	switch (machine) {
	case (EM_NONE):   return "no machine";
	case (EM_M32):    return "AT&T WE 32100"; 
	case (EM_SPACE):  return "SPARC";
	case (EM_386):    return "Intel 80386";
	case (EM_68K):    return "Motorola 68000";
	case (EM_88K):    return "Motorola 88000";
	case (EM_860):    return "Intel 80860";
	case (EM_MIPS):   return "MIPS RS3000";
	default:          return "?";
	}
}
#undef CASE

struct elf32_phdr {
#define PT_NULL     0 // signifies this entry should be ignored
#define PT_LOAD     1 // loadable segment
#define PT_DYNAMIC  2 // dynamic linking information
#define PT_INTERP   3 // pathname to interpreter
#define PT_NOTE     4 // auxiliarry information
#define PT_SHLIB    5 // reserved but unspecified
#define PT_PHDR     6 // program header table
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7fffffff
	elf32_word_t p_type;

	elf32_off_t  p_offset; // offset into file of the region
	elf32_addr_t p_vaddr;  // virtual address to load at
	elf32_addr_t p_paddr;  // ignore
	elf32_word_t p_filesz; // size of the region in the file
	elf32_word_t p_memsz;  // size of the region in memory

#define PF_X   0x1 // execute
#define PF_W   0x2 // write-only
#define PF_R   0x4 // read-only
	elf32_word_t p_flags;

	elf32_word_t p_align; // p_vaddr % p_align == p_offset % p_align
} __attribute__((packed));


struct elf32_file {
	struct vfs_file *file;

	size_t entry;

	struct elf32_phdr *text;
	struct elf32_phdr *data;
	struct elf32_phdr *rodata;
	struct elf32_phdr *bss;

};

#endif /* !__LIB_ELF_ELF32_H__ */
