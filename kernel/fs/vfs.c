/**
 * @file fs/vfs.h
 */
#include <fs/vfs.h>

#include <kernel/mutex.h>
#include <kernel/debug.h>

#include <mm/memory.h>
#include <mm/kmalloc.h>

#include <arch/atomic.h>

#include <stdint.h>
#include <types.h>
#include <string.h>
#include <list.h>
#include <errno.h>

struct vfs_dirent *__vfs_root_dirent = NULL;
struct mutex vfs_mutex = INITIALIZED_MUTEX;

/* this is bullshit */
void vfs_chroot(struct vfs_dirent *root)
{
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
static int dirent_compare_str(struct vfs_dirent *d, const char *name)
{
	return strncmp(d->name, name, VFS_NAMESIZE);
}

/**
 * @brief Find the entry within the directory <d> with name <name>.
 *
 * @return
 *    NULL if the entry does not exist
 *    a pointer to the entry otherwise
 */
static struct vfs_dirent *vfs_find_dirent(struct vfs_dirent *d, const char *name)
{
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
struct vfs_dirent *vfs_get_dirent(char *path)
{
	struct vfs_dirent *found = NULL;
	struct vfs_dirent *d;
	char *cur;
	char *next;

	TRACE("path=%s", path);

	mutex_aquire(&vfs_mutex);

	/*
	 * Luckily our virutal filesystem is simplistic for the time. All dirents
	 * are allocated and can be found by traversing the filesystem tree.
	 */

	if (path[0] != VFS_PATH_DELIM) {
		WARN("%s(path=%s): relative paths are not supported!", __func__, path);
		goto out;
	}

	/*
	 *
	 *
	 * FIXME: This does all the path/string parsing here. I'm sure it has bugs.
	 * Eventually the path parsing should move into a library.
	 *
	 * Bugs:
	 *  - lack of locking dirents
	 *
	 *
	 */
	cur = next = path + 1;
	if (!__vfs_root_dirent) {
		DEBUG("VFS has no root.");
		goto out;
	}

	d = __vfs_root_dirent;
	while (*next) {
		char tmp;
		while (*next && *next != VFS_PATH_DELIM) next++;

		if (!dirent_isdir(d)) {
			WARN("%s is not a directory!", d->name);
			goto out;
		}

		tmp = *next;
		*next = (char) 0;
		d = vfs_find_dirent(d, cur);
		*next = tmp;

		if (!d) {
			WARN("Path %s does not exist starting at %s", path, cur);
			goto out;
		}

		if (!*next) break;
		cur = next = next + 1;
	}

	if (*(next - 1) == VFS_PATH_DELIM && !dirent_isdir(d)) {
		WARN("%s is not a directory. Paths ending in %c should be directories.",
				path, VFS_PATH_DELIM);
		return NULL;
	}

	atomic_inc(&d->refs);
	found = d;

out:
	mutex_release(&vfs_mutex);
	return found;
}

/**
 * @brief Our current VFS implementation allocates all dirents at filesystem
 * startup so we don't free dirents here.
 */
void vfs_put_dirent(struct vfs_dirent *d)
{
	atomic_dec(&d->refs);
}

/**
 * @brief Given a path to a file, create a new vfs_file struct to use
 * the file.
 *
 * @return
 *    A pointer to the allocated file on success
 *    NULL on error
 */
struct vfs_file *new_vfs_file_from_path(char *path)
{
	struct vfs_dirent *dirent;
	struct vfs_file *file;

	dirent = vfs_get_dirent(path);
	if (!dirent)
		return NULL;

	file = kmalloc(sizeof(struct vfs_file));
	if (!file) {
		vfs_put_dirent(dirent);
		return NULL;
	}

	memset(file, 0, sizeof(struct vfs_file));
	file->dirent = dirent;
	file->offset = 0;
	file->fops = dirent->inode->fops;
	file->refs = 1;

	return file;
}

void vfs_file_destroy(struct vfs_file *file)
{
	vfs_put_dirent(file->dirent);

	kfree(file, sizeof(struct vfs_file));
}

void vfs_file_get(struct vfs_file *file)
{
	atomic_inc(&file->refs);
}

/**
 * @brief Release (stop using) a vfs_file. This is not called "vfs_free_file"
 * because we may want to cache the file object for later use.
 */
void vfs_file_put(struct vfs_file *file)
{
	if (atomic_dec(&file->refs))
		return;

	vfs_file_destroy(file);
}

int vfs_open(struct vfs_file *file)
{
	int ret;

	TRACE("file=%p", file);
	ASSERT_NOT_NULL(file);

	mutex_aquire(&vfs_mutex);

	file->offset = 0;
	if (file->fops->open) {
		file->fops->open(file);
	} else {
		VFS_NULL_FOP(open, file);
		ret = -EINVAL;
	}

	mutex_release(&vfs_mutex);
	return ret;
}

void vfs_close(struct vfs_file *file)
{
	TRACE("file=%p", file);
	ASSERT_NOT_NULL(file);

	mutex_aquire(&vfs_mutex);

	if (file->fops->close)
		file->fops->close(file);
	else
		VFS_NULL_FOP(close, file);

	mutex_release(&vfs_mutex);
}

/**
 * @brief Read from a file into a buffer.
 *
 * @param file The vfs_file to read from.
 * @param buf The buffer to read into
 * @param size The number of bytes to read.
 *
 * @return
 *    On success, the number of bytes read is returned.
 *    -EINVAL if reading is not defined for the file
 */
ssize_t vfs_read(struct vfs_file *file, char *buf, size_t size)
{
	ssize_t ret;

	TRACE("file=%p, buf=%p, size=0x%x", file, buf, size);

	mutex_aquire(&vfs_mutex);

	ASSERT_NOT_NULL(file);
	if (!file->fops->read) {
		VFS_NULL_FOP(read, file);
		ret = -EINVAL;
		goto out;
	}

	ret = file->fops->read(file, buf, size, file->offset);

	if (ret > 0) {
		file->offset += ret;
	}

out:
	mutex_release(&vfs_mutex);
	return ret;
}

/**
 * @brief Update the current offset into the file.
 *
 * @return
 *    >= 0 on success (return the offset from the beginning of the file)
 *    -EINVAL if the <whence> parameter is invalid
 *    -EFAULT if the seek request takes the file offset outside the bouds
 *            of the file
 */
ssize_t vfs_seek(struct vfs_file *file, ssize_t offset, int whence)
{
	ssize_t new_offset;
	size_t from;
	ssize_t ret = 0;

	TRACE("file=%p, offset=0x%x, whence=%d", file, offset, whence);
	ASSERT_NOT_NULL(file);

	mutex_aquire(&vfs_mutex);

	switch (whence) {
	case SEEK_SET: from = 0;                           break;
	case SEEK_CUR: from = file->offset;                break;
	case SEEK_END: from = file->dirent->inode->length; break;
	default: ret = -EINVAL; goto out;
	}

	new_offset = from + offset;

	if (new_offset >= 0 && new_offset < (ssize_t) file->dirent->inode->length) {
		file->offset = new_offset;
		ret = new_offset;
		goto out;
	}

	// DON'T CHANGE THIS ERROR CODE. vfs_read_page() uses it.
	ret = -EFAULT;

out:
	mutex_release(&vfs_mutex);
	return ret;
}

ssize_t vfs_read_page(struct vfs_file *file, ssize_t offset, char *page)
{
	ssize_t error;

	error = vfs_seek(file, offset, SEEK_SET);
	if (error < 0) {
		/*
		 * Tried to seek off the end of the file. THIS IS OK. Return 0 bytes read.
		 */
		if (-EFAULT == error) return 0;
		/*
		 * All other error codes are actually errors.
		 */
		return error;
	}

	return vfs_read(file, page, PAGE_SIZE);
}
