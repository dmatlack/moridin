/**
 * @vfs_file fs/vfs.h
 *
 * @brief Virtual vfs_filesystem
 *
 * based on JamesM's tutorial at
 *  http://www.jamesmolloy.co.uk/tutorial_html/8.-The%20VFS%20and%20the%20initrd.html
 * and the Linux Virtual vfs_filesystem
 */
#ifndef __FS_VFS_H__
#define __FS_VFS_H__

#include <types.h>
#include <stdint.h>
#include <debug.h>
#include <list.h>

#define VFS_PATH_DELIM '/'

struct vfs_inode;
struct vfs_dirent;
struct vfs_file;

list_typedef(struct vfs_inode)  vfs_inode_list_t;
list_typedef(struct vfs_dirent) vfs_dirent_list_t;
list_typedef(struct vfs_file)   vfs_file_list_t;

/*
 * Represents a physical file that is stored somewhere.
 */
struct vfs_inode {
  unsigned long inode;

#define VFS_R 0x1
#define VFS_W 0x2
#define VFS_X 0x4
  unsigned int perm;   // permissions

#define VFS_FILE         1
#define VFS_DIRECTORY    2
#define VFS_CHARDEVICE   3
#define VFS_BLOCKDEVICE  4
#define VFS_PIPE         5
#define VFS_SYMLINK      6
#define VFS_TYPE(flags) (flags & 0x7)
#define MOUNTPOINT       8
  unsigned int flags;   // node type

  size_t length;
  vfs_dirent_list_t dirents; // all the dirents referring to this file
};

/*
 * Represents a unique path in the Virtual Filesystem to a given inode.
 * (e.g. two different dirents can point to the same inode, this is called
 * a hard link)
 */
struct vfs_dirent {
#define VFS_NAMESIZE 128
  char name[VFS_NAMESIZE];
  struct vfs_inode *inode;
  struct vfs_dirent *parent;  // containing directory
  vfs_dirent_list_t children; // NULL if vfs_file, list of children if directory
  list_link(struct vfs_dirent) sibling_link; // other vfs_files in the same directory
  list_link(struct vfs_dirent) inode_link; // hardlink brethren
};

static inline bool dirent_isdir(struct vfs_dirent *dirent) {
  return VFS_TYPE(dirent->inode->flags) == VFS_DIRECTORY;
}


/*
 * Represents an open file (may be shared by multiple processes).
 */
struct vfs_file {
  struct vfs_dirent *dirent;
  size_t offset; // read/write offset into the file
  struct vfs_file_ops *ops;
};

struct vfs_file_ops {
  void (*open)(struct vfs_file *);
  void (*close)(struct vfs_file *);

  ssize_t (*read)(struct vfs_file *, char *, size_t size, size_t off);
  ssize_t (*write)(struct vfs_file *, char *, size_t size, size_t off);

  struct vfs_dirent *(*readdir)(struct vfs_file *, unsigned int index);
};

#define VFS_ERROR(_f, _fmt, ...) \
  ERROR("FS vfs_file %s: "_fmt, _f->name, ##__VA_ARGS__)

#define VFS_WARN(_f, _fmt, ...) \
  WARN("FS vfs_file %s: "_fmt, _f->name, ##__VA_ARGS__)

/*
 * 
 * The Virtual vfs_filesystem Inferface
 *
 */
void vfs_chroot(struct vfs_dirent *root);

struct vfs_file *vfs_get_file(char *path);
void vfs_put_file(struct vfs_file *file);

#endif /* !__FS_VFS_H__ */
