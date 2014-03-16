/**
 * @file kernel/loader.c
 *
 * @brief Loading executables into memory.
 */
#include <kernel/loader.h>
#include <mm/kmalloc.h>
#include <kernel/kprintf.h>
#include <kernel/elf.h>

#include <arch/atomic.h>

#include <errno.h>
#include <kernel/debug.h>
#include <assert.h>

#include <fs/vfs.h>
#include <mm/memory.h>
#include <mm/vm.h>

static inline ssize_t read_header(struct vfs_file *file, char *buf, unsigned bufsz) {
  memset(buf, 0, bufsz);
  return vfs_read(file, buf, bufsz);
}

int load_binary(struct vfs_file *file) {
  #define HDRSZ 4 // increase if new binary formats use larger headers
  char header[HDRSZ];
  ssize_t bytes;

  if (!(file->dirent->inode->perm & VFS_X)) {
    WARN("Tried to load non-executable file: %s", file->dirent->name);
    return EPERM;
  }

  /*
   * Read the first few bytes of the file to use to determine the executable
   * format.
   */
  bytes = read_header(file, header, HDRSZ);

  if (is_elf32(header, bytes)) {
    return elf32_load(file);
  }
  // ...
  
  DEBUG("File %s does not match any executable formats.", file->dirent->name);
  return EFAULT;
}
