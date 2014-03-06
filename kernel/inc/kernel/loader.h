/**
 * @file kernel/loader.h
 */
#ifndef __KERNEL_LOADER_H__
#define __KERNEL_LOADER_H__

#include <fs/vfs.h>

int load_binary(struct vfs_file *exec);

#endif /* !__KERNEL_LOADER_H__ */
