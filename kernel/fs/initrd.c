/**
 * @file fs/initrd.h
 *
 */
#include <fs/initrd.h>
#include <fs/vfs.h>
#include <kernel/kmalloc.h>

#include <errno.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <kernel/debug.h>

size_t initrd_location; // set externally

/*
 * initrdfs specific metadata
 */
static struct initrd_hdr *initrd;
static struct initrd_file *initrd_files;

/*
 * All inodes in the ramdisk. There is one inode for each initrd_file.
 *
 * There will also be an inode for the root directory.
 */
static struct vfs_inode *initrd_inodes;
static struct vfs_inode *initrd_root_inode;
static size_t initrd_inodes_size;
static unsigned long initrd_next_inode;

/*
 * All the dirents in the ramdisk. Because ramdisk does not support any
 * fancy features like links, there is one dirent per inode.
 *
 * There will also be a dirent created for the root directory.
 */
static struct vfs_dirent *initrd_dirents;
static struct vfs_dirent *initrd_root_dirent;
static size_t initrd_dirents_size;

/*
 * The file operations supported by initrd.
 */
static struct vfs_file_ops initrd_fops;
static struct vfs_file_ops initrd_root_fops;

int initrd_open(struct vfs_file *f) {
  TRACE("f=%p", f);
  /*
   * Do nothing because the ramdisk is already in memory...
   */
  (void) f;
  return 0;
}

void initrd_close(struct vfs_file *f) {
  TRACE("f=%p");
  /*
   * Do nothing because the ramdisk is already in memory...
   */
  (void) f;
}

ssize_t initrd_read(struct vfs_file *file, char *buf, size_t size,
                    size_t off) {
  struct initrd_file *ramfile = (struct initrd_file *) 
                                file->dirent->inode->object;
  ssize_t bytes;
  char *data;

  TRACE("file=%p, buf=%p, size=0x%x, off=0x%x", file, buf, size, off);

  if (off >= ramfile->length) {
    return 0;
  }

  bytes = umin(size, ramfile->length - off);
  data = (char *) ( ((size_t) initrd) + ramfile->data + off );
  memcpy(buf, data, bytes);
  return bytes;
}

struct vfs_dirent *initrd_readdir(struct vfs_file *f, unsigned int index) {
  ASSERT(dirent_isdir(f->dirent));
  ASSERT_EQUALS(initrd_root_dirent, f->dirent);

  /*
   * + 1 because initrd_dirents[0] is initrd_root_dirent
   */
  return initrd_dirents + 1 + index;
}

static void initrd_init_fops(void) {
  memset(&initrd_fops, 0, sizeof(struct vfs_file_ops));
  initrd_fops.open    = initrd_open;
  initrd_fops.close   = initrd_close;
  initrd_fops.read    = initrd_read;
  initrd_fops.write   = NULL;
  initrd_fops.readdir = NULL;

  memset(&initrd_root_fops, 0, sizeof(struct vfs_file_ops));
  initrd_root_fops.open    = initrd_open;
  initrd_root_fops.close   = initrd_close;
  initrd_root_fops.read    = NULL;
  initrd_root_fops.write   = NULL;
  initrd_root_fops.readdir = initrd_readdir;
}

static void initrd_init_root(struct vfs_dirent *d, struct vfs_inode *i) {
  initrd_root_inode = i;
  initrd_root_dirent = d;

  dirent_init(d, (char *) "");
  d->inode  = i;
  d->parent = d;

  inode_init(i, initrd_next_inode++);
  i->perm   = VFS_R | VFS_X;
  i->flags  = VFS_DIRECTORY;
  i->length = 0;
  i->fops   = &initrd_root_fops;
  i->object = NULL;

  list_insert_tail(&i->dirents, d, hardlink_link);
}

static void initrd_init_file(struct vfs_dirent *d, struct vfs_inode *i,
                             struct initrd_file *ramfile) {
  dirent_init(d, ramfile->name);
  d->inode  = i;
  d->parent = initrd_root_dirent;

  inode_init(i, initrd_next_inode++);
  i->perm   = VFS_R | VFS_X;
  i->flags  = VFS_FILE;
  i->length = ramfile->length;
  i->fops   = &initrd_fops;
  i->object = (void *) ramfile;

  list_insert_tail(&i->dirents, d, hardlink_link);
  list_insert_tail(&initrd_root_dirent->children, d, sibling_link);
}

/**
 * @brief Initialize the initial ramdisk.
 *
 * @param address The address of the ramdisk in memory
 *
 * @return
 *    EINVAL if the address does not point to the initrd
 *    ENOMEM if the kernel runs out of memory
 *
 *    0 on success
 */
void initrd_init(void) {
  struct vfs_inode *cur_inode;
  struct vfs_dirent *cur_dirent;
  unsigned i;

  TRACE();

  initrd = (struct initrd_hdr *) initrd_location;
  initrd_files = (struct initrd_file *) (initrd_location + sizeof(struct initrd_hdr));
  ASSERT_EQUALS(INITRD_MAGIC, initrd->magic);

  kprintf("initrd: 0x%08x\n", initrd);

  initrd_inodes_size = sizeof(struct vfs_inode) * (initrd->nfiles + 1);
  initrd_inodes = kmalloc(initrd_inodes_size);
  ASSERT_NOT_NULL(initrd_inodes);

  initrd_dirents_size = sizeof(struct vfs_dirent) * (initrd->nfiles + 1);
  initrd_dirents = kmalloc(initrd_dirents_size);
  ASSERT_NOT_NULL(initrd_dirents);

  initrd_init_fops();

  initrd_next_inode = 0;
  cur_dirent = initrd_dirents;
  cur_inode = initrd_inodes;

  /*
   * Create an inode and dirent for the "root"
   */
  initrd_init_root(cur_dirent, cur_inode);

  /*
   * Now create a dirent and an inode for every file in th ramdisk
   */
  for (i = 0; i < initrd->nfiles; i++) {
    cur_dirent++;
    cur_inode++;

    initrd_init_file(cur_dirent, cur_inode, initrd_files + i);

    kprintf("  /%-10s: inode=%-2d 0x%06x - 0x%06x\n",
            cur_dirent->name, cur_inode->inode,
            initrd_location + (initrd_files + i)->data,
            initrd_location + (initrd_files + i)->data + cur_inode->length);
  }

  vfs_chroot(initrd_root_dirent);
}

static inline struct initrd_file *initrd_find(struct vfs_file *f) {
  struct initrd_file *r;

  for (r = initrd_files; r < initrd_files + initrd->nfiles; r++) {
    if (!strncmp(f->dirent->name, r->name, umin(VFS_NAMESIZE, INITRD_NAMESIZE))) {
      return r;
    }
  }

  return NULL;
}
