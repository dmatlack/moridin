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
 * @brief Load an executable of the elf32 file format.
 *
 * ASSUMES:
 *  - the vfs_file is already open
 *
 * @return
 *    ENOMEM if the kernel ran out of memory
 *    ENOEXEC if the file could not be parsed
 *    0 on success
 */
int elf32_load(struct vfs_file *file) {
  struct elf32_ehdr *ehdr;

  TRACE("file=%p", file);

  ehdr = elf32_get_ehdr(file);
  if (NULL == ehdr) {
    return ENOMEM;
  }

  return 0;
}

/**
 * @brief Load an executable file.
 *
 * TODO: maybe pass in a process or address space object as well
 * so the loader knows where to load the file.
 *
 * @return
 *    0 on success
 *    EPERM if the file is not executable
 *    EFAULT if the file could not be opened, closed, or read
 *    ENOEXEC if the executable file format was invalid or corrupt
 */
int load(struct vfs_file *file) {
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
    ret = elf32_load(file);
  }
  else {
    DEBUG("File %s does not match any executable formats.",
          file->dirent->name);
    ret = ENOEXEC;
  }

  vfs_close(file);
  return ret;
}
