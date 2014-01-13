/**
 * @file fs/initrd.h
 *
 * @brief Initial Ramdisk
 *
 * The initrd is a very simple, flat, read-only, filesystem. It does not
 * support creating or deleting files.
 *
 * @author David Matlack
 */
#ifndef __FS_INITRD_H__
#define __FS_INITRD_H__

#include <stddef.h>
#include <stdint.h>

struct initrd_hdr {
#define INITRD_MAGIC 0x98119
  uint32_t magic;
  uint32_t nfiles;
};

#define INITRD_NAMESIZE 128
struct initrd_file {
  char name[INITRD_NAMESIZE];
  uint32_t data;
  uint32_t length;
};

int initrd_init(size_t ramdisk);

#endif /* !__FS_INITRD_H__ */
