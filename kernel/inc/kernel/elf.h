/**
 * @file kernel/elf.h
 */
#ifndef __KERNEL_ELF_H__
#define __KERNEL_ELF_H__

#include <kernel/loader.h>
#include <fs/vfs.h>
#include <mm/vm.h>
#include <lib/elf/elf32.h>
#include <string.h>

#define ELF32_MAGIC_SIZE 4
static char elf32_magic[ELF32_MAGIC_SIZE] =
  { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };

static inline bool is_elf32(char *header, size_t bytes) {
  return (bytes >= ELF32_MAGIC_SIZE)
         && 
         (0 == memcmp(header, elf32_magic, ELF32_MAGIC_SIZE));
}

int elf32_parse(struct exec_file *exec, struct vfs_file *file);

int elf32_load(struct vfs_file *file, struct vm_space *space);

#endif /* !__KERNEL_ELF_H__ */
