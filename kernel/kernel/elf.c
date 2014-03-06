/**
 * @file kernel/elf.c
 *
 * @brief Parsing of elf binaries for the kernel.
 *
 */
#include <kernel/elf.h>
#include <kernel/kmalloc.h>

#include <fs/vfs.h>

#include <mm/vm.h>
#include <mm/memory.h>

#include <lib/elf/elf32.h>

#include <stddef.h>
#include <errno.h>
#include <kernel/debug.h>
#include <assert.h>

#include <arch/vm.h>

/**
 * @brief Read the elf32_ehdr from the file.
 *
 * @return 
 *    A pointer to the kmalloc'd header on success
 *    NULL otherwise
 */
struct elf32_ehdr *elf32_get_ehdr(struct vfs_file *file) {
  struct elf32_ehdr *ehdr;
  ssize_t bytes;
  int ret;

  ehdr = kmalloc(sizeof(struct elf32_ehdr));;
  if (NULL == ehdr) return NULL;

  ret = vfs_seek(file, 0, SEEK_SET);
  ASSERT_EQUALS(ret, 0);

  bytes = vfs_read(file, (char *) ehdr, sizeof(struct elf32_ehdr));
  if (bytes < (ssize_t) sizeof(struct elf32_ehdr)) { 
    kfree(ehdr, sizeof(struct elf32_ehdr));
    return NULL;
  }

  return ehdr;
}

/**
 * @brief Read all the elf32_phdr's fromt the file.
 *
 * @return
 *    An array of elf32_phdr's (of length ehdr->e_phnum) on success
 *    NULL otherwise
 */
struct elf32_phdr *elf32_get_phdrs(struct vfs_file *file,
                                   struct elf32_ehdr *ehdr) {
  struct elf32_phdr *phdrs;
  int ret, i;

  ASSERT_EQUALS(ehdr->e_phentsize, sizeof(struct elf32_phdr));

  phdrs = kmalloc(ehdr->e_phentsize * ehdr->e_phnum);
  if (NULL == phdrs) return NULL;

  ret = vfs_seek(file, ehdr->e_phoff, SEEK_SET);
  if (ret < 0) {
    DEBUG("%s: couldn't seek the program headers at 0x%x",
          file->dirent->name, ehdr->e_phoff);
  }

  for (i = 0; i < ehdr->e_phnum; i++) {
    ssize_t bytes = vfs_read(file, (char *) (phdrs + i), ehdr->e_phentsize);
    if (bytes < 0) {
      DEBUG("%s: failed to read the %d phdr (vfs_read() returned %s)",
            file->dirent->name, i, strerr(bytes));
      goto phdrs_cleanup;
    }
    if (bytes < ehdr->e_phentsize) {
      DEBUG("%s: failed to read the _entire_ %d phdr (read %d/%d bytes)",
            file->dirent->name, i, bytes, ehdr->e_phentsize);
      goto phdrs_cleanup;
    }
  }

  return phdrs;
phdrs_cleanup:
  kfree(phdrs, sizeof(struct elf32_phdr) * ehdr->e_phnum);
  return NULL;
}

/**
 * @brief Unload the elf from memory by unmapping the virtual memory and
 * free the physical pages that were backing them.
 */
static void elf32_unmap(struct elf32_ehdr *ehdr, struct elf32_phdr *phdrs) {
  int i, error;
  for (i = 0; i < ehdr->e_phnum; i++) {
    struct elf32_phdr *p = phdrs + i;
    
    error = vm_munmap(p->p_vaddr, p->p_memsz);
    ASSERT_EQUALS(0, error);
  }
}

static void log_phdr(struct elf32_phdr *p) {
  INFO("\n"
       "  %-4s %-10s %-10s %-10s %-6s %-6s %-4s %-6s\n"
       "  %-4d 0x%08x 0x%08x 0x%08x %-6d %-6d 0x%02x 0x%04x",
       "type", "offset", "vaddr", "paddr", "filesz", "memsz", "flg", "align",
       p->p_type, p->p_offset, p->p_vaddr, p->p_paddr, p->p_filesz,
       p->p_memsz, p->p_flags, p->p_align);
}

/**
 * @brief Load the program sections of the elf32 executable into memory
 *
 * @return
 *    0 on success
 */
int __elf32_load(struct vfs_file *file, struct elf32_ehdr *ehdr, struct elf32_phdr *phdrs) {
  unsigned long error;
  int i;

  for (i = 0; i < ehdr->e_phnum; i++) {
    struct elf32_phdr *p = phdrs + i;
    int flags, prot = 0;

    log_phdr(p);

    if (p->p_flags & PF_X) prot |= PROT_EXEC;
    if (p->p_flags & PF_R) prot |= PROT_READ;
    if (p->p_flags & PF_W) prot |= PROT_WRITE;

    flags = MAP_PRIVATE | MAP_FIXED;

    /*
     * Map the elf into memory.
     */
    error = vm_mmap(p->p_vaddr, p->p_memsz, prot, flags, file, p->p_offset);
    error %= PAGE_SIZE;
    if (error) {
      goto load_fail;      
    }

    /*
     * Finally write 0s if the size of the section in memory is larger than
     * the size of the section in the file.
     */
    if (p->p_filesz < p->p_memsz) {
      memset((void *) (p->p_vaddr + p->p_filesz), 0, p->p_memsz - p->p_filesz);
    }
  }

  return 0;

load_fail:
  elf32_unmap(ehdr, phdrs);
  return (int) error;
}

/**
 * @brief Load an executable of the elf32 file format.
 *
 * @return
 *    ENOMEM if the kernel ran out of memory
 *    ENOEXEC if the file could not be loaded
 *    0 on success
 */
int elf32_load(struct vfs_file *file) {
  struct elf32_ehdr *ehdr;
  struct elf32_phdr *phdrs;
  int ret = 0;

  TRACE("file=%p", file);

  ret = vfs_open(file);
  if (ret) {
    return ret;
  }

  /*
   * Read the ELF header
   */
  ehdr = elf32_get_ehdr(file);
  if (NULL == ehdr) {
    ret = ENOMEM;
    goto close_elf_ret;
  }

  /*
   * Set the program counter to be the entrypoint of the elf.
   */
  set_pc(ehdr->e_entry);

  /*
   * Sanity check the executable
   */
  if (ET_EXEC != ehdr->e_type) {
    DEBUG("%s: unsupported type %s", file->dirent->name, elf32_type(ehdr->e_type));
    ret = ENOEXEC;
    goto free_ehdr_ret;
  }
  if (EM_386 != ehdr->e_machine) {
    DEBUG("%s: unsupported machine %s", file->dirent->name,
        elf32_machine(ehdr->e_machine));
    ret = ENOEXEC;
    goto free_ehdr_ret;
  }
  if (ELFDATA2LSB != ehdr->e_ident[EI_DATA]) {
    DEBUG("%s: unsupported big endian", file->dirent->name);
    ret = ENOEXEC;
    goto free_ehdr_ret;
  }
  if (ELFCLASS32 != ehdr->e_ident[EI_CLASS]) {
    DEBUG("%s: unsupported 64-bit elf", file->dirent->name);
    ret = ENOEXEC;
    goto free_ehdr_ret;
  }

  /*
   * Read the program headers
   */
  phdrs = elf32_get_phdrs(file, ehdr);
  if (NULL == ehdr) {
    ret = ENOMEM;
    goto free_ehdr_ret;
  }

  /*
   * Now we load the file into the virtual address space.
   */
  ret = __elf32_load(file, ehdr, phdrs);

  kfree(phdrs, sizeof(struct elf32_phdr) * ehdr->e_phnum);
free_ehdr_ret:
  kfree(ehdr, sizeof(struct elf32_ehdr));
close_elf_ret:
  vfs_close(file);
  return ret;
}
