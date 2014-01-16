/**
 * @file fs/vfs.h
 *
 * based on JamesM's tutorial at
 *  http://www.jamesmolloy.co.uk/tutorial_html/8.-The%20VFS%20and%20the%20initrd.html
 *
 */
#include <fs/vfs.h>

#include <kernel/kmalloc.h>
#include <stdint.h>
#include <types.h>
#include <string.h>
#include <list.h>
#include <debug.h>

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

/**
 * @brief Compare a directory entry with a name string.
 *
 * @return
 *    == 0 if the name matches the entry
 *    <  0 if the entry is lexigraphically less than the name
 *    >  0 otherwise
 */
static int dirent_compare_str(struct vfs_dirent *d, const char *name) {
  return strncmp(d->name, name, VFS_NAMESIZE);
}

/**
 * @brief Find the entry within the directory <d> with name <name>.
 *
 * @return
 *    NULL if the entry does not exist
 *    a pointer to the entry otherwise
 */
static struct vfs_dirent *vfs_find_dirent(struct vfs_dirent *d, const char *name) {
  struct vfs_dirent *entry;

  ASSERT(dirent_isdir(d));

  list_foreach(entry, &d->children, sibling_link) {
    if (0 == dirent_compare_str(entry, name)) {
      return entry;
    }
  }

  return NULL;
}

/**
 * @brief Get the vfs_dirent struct identified by the string <path>.
 *
 * @return NULL on error (e.g. invalid path)
 */
struct vfs_dirent *vfs_get_dirent(char *path) {
  struct vfs_dirent *d;
  char *cur;
  char *next;

  TRACE("path=%s", path);

  /*
   * Luckily our virutal filesystem is simplistic for the time. All dirents
   * are allocated and can be found by traversing the filesystem tree.
   */

  if (path[0] != VFS_PATH_DELIM) {
    WARN("%s(path=%s): relative paths are not supported!", __func__, path);
    return NULL;
  }

  /*
   *
   *
   * FIXME: This does all the path/string parsing here. I'm sure it has bugs.
   *
   * Eventually the path parsing should move into a library.
   *
   *
   */
  cur = next = path + 1;
  d = __vfs_root_dirent;
  ASSERT(dirent_isdir(d));
  while (*next) {
    char tmp;
    while (*next && *next != VFS_PATH_DELIM) next++;

    if (!dirent_isdir(d)) {
      WARN("%s is not a directory!", d->name);
      return NULL;
    }

    tmp = *next;
    *next = (char) 0;
    d = vfs_find_dirent(d, cur);
    *next = tmp;

    if (!d) {
      WARN("Path %s does not exist starting at %s", path, cur);
      return NULL;
    }

    if (!*next) break;
    cur = next = next + 1;
  }
  
  if (*(next - 1) == VFS_PATH_DELIM && !dirent_isdir(d)) {
    WARN("%s is not a directory. Paths ending in %c should be directories.",
         path, VFS_PATH_DELIM);
    return NULL;
  }

  return d;
}

/**
 * @brief Our current VFS implementation allocates all dirents at filesystem
 * startup so we don't free dirents here.
 */
void vfs_put_dirent(struct vfs_dirent *d) {
  (void) d;
}

/**
 * @brief Given a path to a file, create a new vfs_file struct to use
 * the file.
 *
 * @return
 *    A pointer to the allocated file on success
 *    NULL on error
 */
struct vfs_file *vfs_get_file(char *path) {
  struct vfs_dirent *dirent;
  struct vfs_file *file;

  dirent = vfs_get_dirent(path);
  if (!dirent) {
    return NULL;
  }

  file = kmalloc(sizeof(struct vfs_file));
  if (!file) {
    vfs_put_dirent(dirent);
    return NULL;
  }

  memset(file, 0, sizeof(struct vfs_file));
  file->dirent = dirent;
  file->offset = 0;
  file->ops = NULL; //FIXME how to get the ops... ?

  return file;
}

/**
 * @brief Release (stop using) a vfs_file. This is not called "vfs_free_file"
 * because we may want to cache the file object for later use.
 */
void vfs_put_file(struct vfs_file *file) {
  vfs_put_dirent(file->dirent);
  kfree(file, sizeof(struct vfs_file));
}
