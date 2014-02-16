/**
 * @file kernel/loader.c
 *
 * @brief Loading executables into memory.
 */
#include <kernel/atomic.h>
#include <kernel/loader.h>
#include <kernel/kmalloc.h>
#include <kernel/kprintf.h>
#include <kernel/elf.h>

#include <errno.h>
#include <debug.h>
#include <assert.h>

#include <fs/vfs.h>
#include <mm/memory.h>
#include <mm/vm.h>

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
int load(struct exec_file *exec, struct vm_space *space) {
  int ret;

  ret = exec->load(exec->file, space);

  return ret;
}

struct exec_file *exec_file_get(struct vfs_file *file) {
  struct exec_file *exec;
  int ret = 0;

  exec = kmalloc(sizeof(struct exec_file));
  if (NULL == exec) return NULL;

  ret = exec_file_init(exec, file);
  if (ret) {
    kfree(exec, sizeof(struct exec_file));
    exec = NULL;
  }

  return exec;
}

struct exec_file *exec_file_copy(struct exec_file *exec) {
  atomic_add(&exec->refs, 1);
  return exec;
}

void exec_file_put(struct exec_file *exec) {
  atomic_add(&exec->refs, 1);

  if (0 == exec->refs) {
    vfs_file_put(exec->file);
    kfree(exec, sizeof(struct exec_file));
  }
}

/**
 * @brief Initialize an exec_file struct.
 */
int exec_file_init(struct exec_file *exec, struct vfs_file *file) {
#define LOAD_HEADER_SIZE 4
  char buffer[LOAD_HEADER_SIZE];
  ssize_t bytes;
  int ret = 0;

  exec->file = file;
  exec->entry = 0;
  exec->refs = 1;

  if (!(file->dirent->inode->perm & VFS_X)) {
    WARN("Tried to load non-executable file: %s", file->dirent->name);
    return EPERM;
  }

  ret = vfs_open(file);
  if (ret < 0) {
    DEBUG("Could not open %s: %d/%s", file->dirent->name, ret, strerr(ret));
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
   * Invoke the correct parser based on the file's executable format
   */
  if (is_elf32(buffer, bytes)) {
    exec->load = elf32_load;
    ret = elf32_parse(exec, file);
  }
  else {
    DEBUG("File %s does not match any executable formats.",
          file->dirent->name);
    ret = ENOEXEC;
  }

  vfs_close(file);
  return ret;
}
