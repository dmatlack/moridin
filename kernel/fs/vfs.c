/**
 * @file fs/vfs.h
 *
 * based on JamesM's tutorial at
 *  http://www.jamesmolloy.co.uk/tutorial_html/8.-The%20VFS%20and%20the%20initrd.html
 *
 */
#include <fs/vfs.h>

#include <stdint.h>
#include <types.h>

struct vfs_dirent *__vfs_root_dirent;

/**
 * @brief Set the root of the filesystem. This is like "mounting" a filesystem
 * at /, except a really bad version of "mounting" :)
 *
 * After calling this function, the filesystem rooted at <root>, will be the VFS.
 */
void vfs_chroot(struct vfs_dirent *root) {
  __vfs_root_dirent = root;
}
