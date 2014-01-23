/**
 * @file kernel/loader.h
 */
#ifndef __KERNEL_LOADER_H__
#define __KERNEL_LOADER_H__

#include <mm/vm.h>

int load(struct vfs_file *file, struct vm_space *space);

#endif /* !__KERNEL_LOADER_H__ */
