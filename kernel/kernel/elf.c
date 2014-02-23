/**
 * @file kernel/elf.c
 *
 * @brief Parsing of elf binaries for the kernel.
 *
 * @author David Matlack
 */
#include <kernel/elf.h>
#include <kernel/kmalloc.h>

#include <fs/vfs.h>
#include <mm/vm.h>

#include <lib/elf/elf32.h>

#include <stddef.h>
#include <errno.h>
#include <kernel/debug.h>
#include <assert.h>

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

static inline vm_flags_t elf32_to_vm_flags(elf32_word_t p_flags) {
  vm_flags_t v = 0;
  v |= (p_flags & PF_X) ? VM_X : 0;
  v |= (p_flags & PF_R) ? VM_R : 0;
  v |= (p_flags & PF_W) ? VM_W : 0;
  return v | VM_U;
}

/**
 * @brief Load the program section into the virtual address space.
 *
 * @warning This function assumes <space> is the currently in-use vm_space.
 *
 * @return
 *    0 on success
 *    EINVAL if the header is not a loadable section
 *    EFAULT if the section could not be read from the file
 *    ENOMEM if we ran out of memory while trying to load the section
 *    other non-0 error codes if virtual memory mapping or file reading fails
 */
int elf32_load_phdr(struct vfs_file *file, struct elf32_phdr *phdr,
                    char *buf, int bufsz) {
  ssize_t bytes;
  size_t offset;
  int ret;

  if (PT_LOAD != phdr->p_type) {
    WARN("%s: Non loadable elf32 program section type: %d", file->dirent->name,
         phdr->p_type);
    return EINVAL;
  }

  offset = 0;
  if (phdr->p_filesz > 0) {
    ret = vfs_seek(file, phdr->p_offset, SEEK_SET);
    if (ret != (int) phdr->p_offset) {
      return EFAULT;
    }

    /*
     * Copy the program section contents from the file,
     * into memory.
     */
    do {
      bytes = vfs_read(file, buf, bufsz);
      if (bytes < 0) {
        return bytes;
      }
      memcpy((void *) (phdr->p_vaddr + offset), buf, bytes);

      offset += bytes;
    } while (offset < phdr->p_filesz);
  }

  /*
   * Finally write 0s if the size of the section in memory is
   * larger than the size of the section in the file.
   */
  if (offset < phdr->p_memsz) {
    memset((void *) (phdr->p_vaddr + offset), 0, phdr->p_memsz - offset);
  }

  return 0;
}

/**
 * @brief Unload the elf from memory by unmapping the virtual memory and
 * free the physical pages that were backing them.
 */
void __elf32_unload(struct vm_space *space, struct elf32_ehdr *ehdr,
                    struct elf32_phdr *phdrs) {
  int i;
  for (i = 0; i < ehdr->e_phnum; i++) {
    struct elf32_phdr *p = phdrs + i;
    vm_unmap(space, p->p_vaddr, p->p_memsz);
    tlb_invalidate(p->p_vaddr, p->p_memsz);
  }
}

/**
 * @brief Map the elf program section into virtual memory.
 */
int elf32_map_ps(struct vm_space *space, struct elf32_phdr *phdr) {
  int ret;
  int flags;

  flags = elf32_to_vm_flags(phdr->p_flags);

  /*
   * Map the region into virtual memory.
   */
  ret = vm_map(space, phdr->p_vaddr, phdr->p_memsz, flags);
  if (ret) return ret;
  
  /*
   * Flush these pages from the TLB so we don't page fault trying to write
   * to them.
   */
  tlb_invalidate(phdr->p_vaddr, phdr->p_memsz);

  return 0;
}

/**
 * @brief Load the program sections of the elf32 executable into memory
 *
 * @return
 *    0 on success
 */
int __elf32_load(struct vfs_file *file, struct vm_space *space,
                 struct elf32_ehdr *ehdr, struct elf32_phdr *phdrs) {
  void *old_space_object;
  char *buf;
  int bufsz;
  int ret;
  int i;

  bufsz = 1024;
  buf = kmalloc(bufsz);
  if (NULL == buf) {
    return ENOMEM;
  }

  // TODO we could probably get the old_space_object's corresponding vm_space
  // struct by examining the currently running thread.
  old_space_object = __vm_space_switch(space->object);

  for (i = 0; i < ehdr->e_phnum; i++) {
    struct elf32_phdr *p = phdrs + i;

    INFO("%s: elf32_phdr %d\n"
         "  %-4s %-10s %-10s %-10s %-6s %-6s %-4s %-6s\n"
         "  %-4d 0x%08x 0x%08x 0x%08x %-6d %-6d 0x%02x 0x%04x",
         file->dirent->name, i,
         "type", "offset", "vaddr", "paddr", "filesz", "memsz", "flg", "align",
         p->p_type, p->p_offset, p->p_vaddr, p->p_paddr, p->p_filesz,
         p->p_memsz, p->p_flags, p->p_align);

    /*
     * Map the memory needed by the program section into virtual memory
     */
    ret = elf32_map_ps(space, p);
    if (ret) {
      __elf32_unload(space, ehdr, phdrs);
      goto __elf32_load_cleanup;
    }

    /*
     * Copy the file contents into memory
     */
    ret = elf32_load_phdr(file, p, buf, bufsz);
    if (ret) {
      __elf32_unload(space, ehdr, phdrs);
      goto __elf32_load_cleanup;
    }
  }

  ret = 0;
__elf32_load_cleanup:
  __vm_space_switch(old_space_object);
  kfree(buf, bufsz);
  return ret;
}

/**
 * @brief Load an executable of the elf32 file format.
 *
 * @return
 *    ENOMEM if the kernel ran out of memory
 *    ENOEXEC if the file could not be loaded
 *    0 on success
 */
int elf32_load(struct vfs_file *file, struct vm_space *space) {
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
    goto free_ehdr_ret;
  }
  if (ELFCLASS32 != ehdr->e_ident[EI_CLASS]) {
    DEBUG("%s: unsupported 64-bit elf", file->dirent->name);
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
  ret = __elf32_load(file, space, ehdr, phdrs);

  kfree(phdrs, sizeof(struct elf32_phdr) * ehdr->e_phnum);
free_ehdr_ret:
  kfree(ehdr, sizeof(struct elf32_ehdr));
close_elf_ret:
  vfs_close(file);
  return ret;
}

/**
 * @brief Parse the ELF file referenced by <file> and build the exec_file
 * struct based on the contents.
 */
int elf32_parse(struct exec_file *exec, struct vfs_file *file) {
  struct elf32_ehdr *ehdr;

  ehdr = elf32_get_ehdr(file);
  if (NULL == ehdr) {
    return ENOMEM;
  }

  exec->entry = ehdr->e_entry;

  kfree(ehdr, sizeof(struct elf32_ehdr));
  return 0;
}
