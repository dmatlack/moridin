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

struct vfs_node *vfs_root;

static inline bool isdir(struct vfs_node *f) {
  return VFS_TYPE(f->flags) == VFS_DIRECTORY;
}

void vfs_open(struct vfs_node *f) {
  if (f->open) {
    f->open(f);
  }
  else {
    VFS_ERROR(f, "Missing open()");
  }
}

void vfs_close(struct vfs_node *f) {
  if (f->close) {
    f->close(f);
  }
  else {
    VFS_ERROR(f, "Missing close()");
  }
}

uint32_t vfs_read(struct vfs_node *f, uint32_t off, uint32_t size, uint8_t *buf) {
  if (f->read) {
    return f->read(f, off, size, buf);
  }
  else {
    VFS_ERROR(f, "Missing read()");
    return 0;
  }
}

uint32_t vfs_write(struct vfs_node *f, uint32_t off, uint32_t size, uint8_t *buf) {
  if (!(f->perm & VFS_W)) {
    VFS_WARN(f, "Attempted write() on read-only file");
    return 0;
  }

  if (f->write) {
    return f->write(f, off, size, buf);
  }
  else {
    VFS_ERROR(f, "Missing write()");
    return 0;
  }
}

struct dirent *vfs_readdir(struct vfs_node *f, uint32_t index) {
  if (!isdir(f)) {
    VFS_WARN(f, "Attempted readdir() but not a directory");
    return NULL;
  }

  if (f->readdir) {
    return f->readdir(f, index);
  }
  else {
    VFS_ERROR(f, "Missing readdir()");
    return NULL;
  }
}

struct vfs_node *vfs_finddir(struct vfs_node *f, char* name) {
  if (!isdir(f)) {
    VFS_WARN(f, "Attempted finddir() but not a directory");
    return NULL;
  }

  if (f->finddir) {
    return f->finddir(f, name);
  }
  else {
    VFS_ERROR(f, "Missing readdir()");
    return NULL;
  }

}
