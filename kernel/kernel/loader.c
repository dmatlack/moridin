/**
 * @file kernel/loader.c
 *
 * @brief Loading executables into memory.
 */
#include <kernel/kmalloc.h>
#include <kernel/kprintf.h>

#include <errno.h>
#include <debug.h>
#include <assert.h>

#include <fs/vfs.h>
#include <lib/elf/elf32.h>
#include <mm/memory.h>
#include <mm/vm.h>

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
  v |= p_flags & PF_X ? VM_X : 0;
  v |= p_flags & PF_R ? VM_R : 0;
  v |= p_flags & PF_W ? VM_W : 0;
  return v;
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
  int i;

  // TODO we could probably get the old_space_object's corresponding vm_space
  // struct by examining the currently running thread.
  old_space_object = __vm_space_switch(space->object);

  for (i = 0; i < ehdr->e_phnum; i++) {
    struct elf32_phdr *p = phdrs + i;

    INFO("%s: elf32 phdr %d\n"
         "  %-4s %-10s %-10s %-10s %-6s %-6s %-4s %-6s\n"
         "  %-4d 0x%08x 0x%08x 0x%08x %-6d %-6d 0x%02x 0x%04x",
         file->dirent->name, i,
         "type", "offset", "vaddr", "paddr", "filesz", "memsz", "flg", "align",
         p->p_type, p->p_offset, p->p_vaddr, p->p_paddr, p->p_filesz,
         p->p_memsz, p->p_flags, p->p_align);
  }

  __vm_space_switch(old_space_object);

  return 0;
}

/**
 * @brief Load an executable of the elf32 file format.
 *
 * ASSUMES:
 *  - the vfs_file is already open
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

  /*
   * Read the ELF header
   */
  ehdr = elf32_get_ehdr(file);
  if (NULL == ehdr) {
    return ENOMEM;
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

  goto free_phdrs_ret;
free_phdrs_ret:
  kfree(phdrs, sizeof(struct elf32_phdr) * ehdr->e_phnum);
free_ehdr_ret:
  kfree(ehdr, sizeof(struct elf32_ehdr));
  return ret;
}

/**
 * @brief Load an executable file.
 *
 * TODO: maybe pass in a process or address space object as well
 * so the loader knows where to load the file.
 *
 * @param file The file object of the executable to load
 * @param space The virtual address space into which to load the
 * file.
 *
 * @return
 *    0 on success
 *    EPERM if the file is not executable
 *    EFAULT if the file could not be opened, closed, or read
 *    ENOEXEC if the executable file format was invalid or corrupt
 */
int load(struct vfs_file *file, struct vm_space *space) {
#define LOAD_HEADER_SIZE 4
  char buffer[LOAD_HEADER_SIZE];
  ssize_t bytes;
  int ret = 0;

  TRACE("file=%p", file);

  if (!(file->dirent->inode->perm & VFS_X)) {
    WARN("Tried to load non-executable file: %s", file->dirent->name);
    return EPERM;
  }

  ret = vfs_open(file);
  if (ret < 0) {
    DEBUG("Could not open %s for loading: %d/%s", file->dirent->name,
          ret, strerr(ret));
    return ret;
  }

  /*
   * Read the first few bytes of the file to determine the executable
   * format
   */
  bytes = vfs_read(file, buffer, LOAD_HEADER_SIZE);
  if (bytes < LOAD_HEADER_SIZE) {
    ret = bytes;
    DEBUG("Could not read %d bytes from file %s (returned: %d/%s)",
          LOAD_HEADER_SIZE, file->dirent->name, ret, strerr(ret));
    vfs_close(file);
    return ret;
  }

  /*
   * Invoke the correct loader based on the file's executable format
   */
  if (bytes >= ELF32_MAGIC_SIZE &&
      0 == memcmp(buffer, elf32_magic, ELF32_MAGIC_SIZE)) {
    ret = elf32_load(file, space);
  }
  else {
    DEBUG("File %s does not match any executable formats.",
          file->dirent->name);
    ret = ENOEXEC;
  }

  vfs_close(file);
  return ret;
}
