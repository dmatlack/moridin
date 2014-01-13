/**
 * @file fs/vfs.h
 *
 * @brief Virtual Filesystem
 *
 * based on JamesM's tutorial at
 *  http://www.jamesmolloy.co.uk/tutorial_html/8.-The%20VFS%20and%20the%20initrd.html
 *
 */
#ifndef __FS_VFS_H__
#define __FS_VFS_H__

#include <stdint.h>
#include <debug.h>

extern struct vfs_node *vfs_root;

#define VFS_NAMESIZE 128

struct dirent {
  char name[VFS_NAMESIZE];
  uint32_t inode;
};

struct vfs_node {
  char name[VFS_NAMESIZE];

#define VFS_R 0x1
#define VFS_W 0x2
#define VFS_X 0x4
  uint32_t perm;   // permissions

#define VFS_FILE         1
#define VFS_DIRECTORY    2
#define VFS_CHARDEVICE   3
#define VFS_BLOCKDEVICE  4
#define VFS_PIPE         5
#define VFS_SYMLINK      6
#define VFS_TYPE(flags) (flags & 0x7)
#define MOUNTPOINT       8
  uint32_t flags;   // node type
  uint32_t inode;   // device-specific identification number
  uint32_t length;  // size of file in bytes


  void (*open)(struct vfs_node *f);
  void (*close)(struct vfs_node *f);

  uint32_t (*read)(struct vfs_node *f, uint32_t off, uint32_t size, uint8_t *buf);
  uint32_t (*write)(struct vfs_node *f, uint32_t off, uint32_t size, uint8_t *buf);

  struct dirent *(*readdir)(struct vfs_node *f, uint32_t index);
  struct vfs_node *(*finddir)(struct vfs_node *d, char* name);


  struct vfs_node *ptr; // symbolic links
};

#define VFS_ERROR(_f, _fmt, ...) \
  ERROR("FS file %s: "_fmt, _f->name, ##__VA_ARGS__)

#define VFS_WARN(_f, _fmt, ...) \
  WARN("FS file %s: "_fmt, _f->name, ##__VA_ARGS__)

/*
 * 
 * The Virtual Filesystem Inferface
 *
 */
void vfs_open(struct vfs_node *f);
void vfs_close(struct vfs_node *f);
uint32_t vfs_read(struct vfs_node *f, uint32_t off, uint32_t size, uint8_t *buf);
uint32_t vfs_write(struct vfs_node *f, uint32_t off, uint32_t size, uint8_t *buf);
struct dirent *vfs_readdir(struct vfs_node *f, uint32_t index);
struct vfs_node *vfs_finddir(struct vfs_node *d, char* name);

#endif /* !__FS_VFS_H__ */
