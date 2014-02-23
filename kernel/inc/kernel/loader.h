/**
 * @file kernel/loader.h
 */
#ifndef __KERNEL_LOADER_H__
#define __KERNEL_LOADER_H__

#include <fs/vfs.h>
#include <mm/vm.h>

struct exec_file {
  struct vfs_file *file;
  size_t entry;
  int refs;

  //
  // Maybe add the address of the different sections
  //  .text
  //  .data
  //  .bss
  //  .rodata
  //   ...
  //

  /**
   * @brief Load this executable file into the provided virtual memory
   * address space.
   */
  int (*load)(struct vfs_file *file, struct vm_space *space);
};

int load(struct exec_file *file, struct vm_space *space);

int exec_file_init(struct exec_file *exec, struct vfs_file *file);

void exec_file_copy(struct exec_file *to, struct exec_file *from);

#endif /* !__KERNEL_LOADER_H__ */
