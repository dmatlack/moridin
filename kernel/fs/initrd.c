/**
 * @file fs/initrd.h
 *
 * @author David Matlack
 */
#include <fs/initrd.h>
#include <fs/vfs.h>

#include <kernel.h>
#include <errno.h>
#include <string.h>
#include <math.h>

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
struct vfs_file_ops initrd_fops;

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
int initrd_init(size_t address) {
  struct vfs_inode *cur_inode;
  struct vfs_dirent *cur_dirent;
  unsigned i;

  initrd = (struct initrd_hdr *) address;
  initrd_files = (struct initrd_file *) (address + sizeof(struct initrd_hdr));

  if (INITRD_MAGIC != initrd->magic) {
    return EINVAL;
  }

  initrd_inodes_size = sizeof(struct vfs_inode) * initrd->nfiles + 1;
  initrd_inodes = kmalloc(initrd_inodes_size);
  if (NULL == initrd_inodes) return ENOMEM;

  initrd_dirents_size = sizeof(struct vfs_dirent) * (initrd->nfiles + 1);
  initrd_dirents = kmalloc(initrd_dirents_size);
  if (NULL == initrd_dirents) {
    kfree(initrd_inodes, initrd_inodes_size);
    return ENOMEM;
  }

  initrd_next_inode = 0;

  /*
   * Create an inode and dirent for the "root"
   */
  cur_dirent = initrd_dirents;
  cur_inode = initrd_inodes;

  cur_dirent->name[0] = (char) 0;
  cur_dirent->inode   = cur_inode;
  cur_dirent->parent  = cur_dirent;
  list_init(&cur_dirent->children);
  list_elem_init(cur_dirent, sibling_link);
  list_elem_init(cur_dirent, inode_link);

  cur_inode->inode = initrd_next_inode++;
  cur_inode->perm = VFS_R;
  cur_inode->flags = VFS_DIRECTORY;
  cur_inode->length = 0;
  list_init(&cur_inode->dirents);

  list_insert_tail(&cur_inode->dirents, cur_dirent, inode_link);
  initrd_root_inode = cur_inode;
  initrd_root_dirent = dirent;

  /*
   * Now create a dirent and an inode for every file in th ramdisk
   */
  kprintf("initrd: ");
  for (i = 0; i < initrd->nfiles; i++) {
    struct initrd_file *ramfile = initrd_files + i;

    cur_inode++;
    cur_dirent++;

    memcpy(cur_dirent->name, ramfile->name, min(VFS_NAMESIZE, INITRD_NAMESIZE));
    cur_dirent->name[VFS_NAMESIZE-1] = (char) 0;
    cur_dirent->inode   = cur_inode;
    cur_dirent->parent  = initrd_root_dirent;
    list_init(&cur_dirent->children);
    list_elem_init(cur_dirent, sibling_link);
    list_elem_init(cur_dirent, inode_link);

    cur_inode->inode = initrd_next_inode++;
    cur_inode->perm = VFS_R;
    cur_inode->flags = VFS_FILE;
    cur_inode->length = ramfile->length;
    list_init(&cur_inode->dirents);

    list_insert_tail(&cur_inode->dirents, cur_dirent, inode_link);
    list_insert_tail(&initrd_root_dirent->children, cur_dirent, sibling_link);

    kprintf("%s ", cur_dirent->name);
  }
  kprintf("\n");

  memset(&initrd_fops, 0, sizeof(file_ops));
  initrd_fops.open = initrd_open;
  initrd_fops.close = initrd_close;
  initrd_fops.read = initrd_read;
  initrd_fops.write = NULL;
  initrd_fops.readdir = initrd_readdir;

  vfs_chroot(&initrd_vroot);
  return 0;
}

static inline struct initrd_file *initrd_find(struct vfs_file *f) {
  struct initrd_file *r;

  for (r = initrd_files; r < initrd_files + initrd->nfiles; r++) {
    if (!strncmp(f->name, r->name, min(VFS_NAMESIZE, INITRD_NAMESIZE))) {
      return r;
    }
  }

  return NULL;
}

/**
 * @brief Open a file in the initial ramdisk. This function does
 * nothing because the initial ramdisk is already in memory.
 */
void initrd_open(struct vfs_file *f) {
  TRACE("f=%p", f);
}

/**
 * @brief Close a file in the initial ramdisk. This function does
 * nothing because the initail ramdisk is already in memory.
 */
void initrd_close(struct vfs_file *f) {
  TRACE("f=%p");
}

ssize_t initrd_read(struct vfs_file *f, char *buf, size_t size, size_t off) {
  //TODO
  return -1;
}

struct dirent *initrd_readdir(struct vfs_file *f, unsigned int index) {
  if (VFS_TYPE(f->dirent-inode->flags) != VFS_DIRECTORY) {
    return NULL;
  }
}
