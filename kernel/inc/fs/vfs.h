/**
 * @vfs_file fs/vfs.h
 *
 * @brief Virtual vfs_filesystem
 *
 * based on JamesM's tutorial at
 *  http://www.jamesmolloy.co.uk/tutorial_html/8.-The%20VFS%20and%20the%20initrd.html
 *
 * and the Linux Virtual Filesystem
 */
#ifndef __FS_VFS_H__
#define __FS_VFS_H__

#include <types.h>
#include <stdint.h>
#include <debug.h>
#include <list.h>
#include <string.h>

#define VFS_PATH_DELIM '/'

struct vfs_inode;
struct vfs_dirent;
struct vfs_file;
struct vfs_file_ops;

list_typedef(struct vfs_inode)  vfs_inode_list_t;
list_typedef(struct vfs_dirent) vfs_dirent_list_t;
list_typedef(struct vfs_file)   vfs_file_list_t;

/*
 * Represents a physical file that is stored somewhere. For each file, there
 * is at most one vfs_inode object globally accross the entire kernel at any
 * time.
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
  struct vfs_file_ops *fops;
  vfs_dirent_list_t dirents; // all the dirents referring to this file
  void *object; // a pointer to private data (to be used by the underlying fs)
};

/*
 * Represents a unique path in the Virtual Filesystem to a given inode.
 * (e.g. two different dirents can point to the same inode, this is called
 * a hard link). For each file, there will be [0,N] vfs_dirents in existence
 * at any time, where N is the number of hardlinks pointing to the file
 * globally.
 */
struct vfs_dirent {
#define VFS_NAMESIZE 128
  char name[VFS_NAMESIZE];
  struct vfs_inode *inode;
  struct vfs_dirent *parent;  // containing directory
  vfs_dirent_list_t children; // NULL if vfs_file, list of children if directory
  list_link(struct vfs_dirent) sibling_link; // other vfs_files in the same directory
  list_link(struct vfs_dirent) hardlink_link; // hardlink brethren
  unsigned refs; // reference counter
};

/*
 * Represents an open file (may be shared by multiple processes). For each file
 * there can be any number of vfs_file objects, since files can be opened
 * by many processes.
 */
struct vfs_file {
  struct vfs_dirent *dirent;
  size_t offset; // read/write offset into the file
  struct vfs_file_ops *fops;
  unsigned refs; // reference counter
};

/*
 * The operations that can be performed on a file.
 */
struct vfs_file_ops {
  int (*open)(struct vfs_file *);
  void (*close)(struct vfs_file *);

  /**
   * @brief Read <size> bytes starting at <off> in <f> into <buf>.
   *
   * @return
   *    < 0 code on error
   *      TODO list possible error codes
   *    the number of bytes read on success
   */
  ssize_t (*read)(struct vfs_file *f, char *buf, size_t size, size_t off);

  ssize_t (*write)(struct vfs_file *, char *, size_t size, size_t off);

  struct vfs_dirent *(*readdir)(struct vfs_file *, unsigned int index);
};

#define VFS_ERROR(_fmt, ...) \
  ERROR("%d: "_fmt, __func__, ##__VA_ARGS__)

#define VFS_WARN(_fmt, ...) \
  ERROR("%d: "_fmt, __func__, ##__VA_ARGS__)

#define VFS_NULL_FOP(_fop, file) \
    VFS_WARN("fop %s() is NULL for file %s.", \
             #_fop, (file)->dirent->name)

static inline void dirent_init(struct vfs_dirent *d, char *name) {
  memset(d, 0, sizeof(struct vfs_dirent));

  strncpy(d->name, name, VFS_NAMESIZE);
  d->name[VFS_NAMESIZE-1] = (char) 0;

  list_init(&d->children);
  list_elem_init(d, sibling_link);
  list_elem_init(d, hardlink_link);
}

static inline bool dirent_isdir(struct vfs_dirent *dirent) {
  return VFS_TYPE(dirent->inode->flags) == VFS_DIRECTORY;
}

static inline void inode_init(struct vfs_inode *i, unsigned long inode) {
  memset(i, 0, sizeof(struct vfs_inode));
  i->inode = inode;
  list_init(&i->dirents);
}

#define SEEK_SET 0 // offset from beginning of file
#define SEEK_CUR 1 // offset from current position
#define SEEK_END 2 // offset from the end of the file
//#define SEEK_DATA 3
//#define SEEK_HOLE 4

/*
 * 
 * The Virtual vfs_filesystem Inferface
 *
 */
void vfs_chroot(struct vfs_dirent *root);

struct vfs_file *vfs_file_get(char *path);
void vfs_file_put(struct vfs_file *file);

int     vfs_open  (struct vfs_file *file);
void    vfs_close (struct vfs_file *file);
ssize_t vfs_read  (struct vfs_file *file, char *buf, size_t size);
ssize_t vfs_seek  (struct vfs_file *file, ssize_t offset, int whence);

#endif /* !__FS_VFS_H__ */
