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

struct vfs_node initrd_root;
struct vfs_node *vnodes;

static struct initrd_hdr *initrd;
static struct initrd_file *initrd_files;

/**
 * @brief Initialize the initial ramdisk.
 *
 * @param address The address of the ramdisk
 *
 * @return
 *    EINVAL if the address does not point to the initrd
 *    ENOMEM if the kernel runs out of memory
 *
 *    0 on success
 */
int initrd_init(size_t address) {
  unsigned i;

  initrd = (struct initrd_hdr *) address;
  initrd_files = (struct initrd_file *) (address + sizeof(struct initrd_hdr));

  if (INITRD_MAGIC != initrd->magic) {
    return EINVAL;
  }

  vnodes = kmalloc(sizeof(struct vfs_node) * initrd->nfiles);
  if (NULL == vnodes) return ENOMEM;

  initrd_root.name[0] = (char) 0;
  initrd_root.perm = VFS_R;
  initrd_root.flags = VFS_DIRECTORY;
  initrd_root.inode = 0;
  initrd_root.length = 0;
  initrd_root.open = NULL;
  initrd_root.close = NULL;
  initrd_root.read = NULL;
  initrd_root.write= NULL;
  initrd_root.readdir = NULL; //FIXME
  initrd_root.finddir = NULL; //FIXME

  kprintf("initrd: ");
  for (i = 0; i < initrd->nfiles; i++) {
    struct vfs_node *vnode = vnodes + i;
    struct initrd_file *rnode = initrd_files + i;

    memcpy(vnode->name, rnode->name, strlen(rnode->name) + 1);

    vnode->perm = VFS_R;
    vnode->flags = VFS_FILE;
    vnode->inode = i+1;
    vnode->length = rnode->length;
    vnode->open = NULL;
    vnode->close = NULL;
    vnode->read = NULL;
    vnode->write= NULL;
    vnode->readdir = NULL; //FIXME
    vnode->finddir = NULL; //FIXME

    kprintf("%s ", vnode->name);
  }
  kprintf("\n");

  vfs_root = &initrd_root;
  return 0;
}
