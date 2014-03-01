/**
 * @file fs/initrd.h
 *
 * @brief Initial Ramdisk
 *
 * The initrd is a very simple, flat, read-only, filesystem. It does not
 * support creating or deleting files.
 *
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

struct initrd_file {
#define INITRD_NAMESIZE 128
  char name[INITRD_NAMESIZE];
  /*
   * The location of the files data as an offset from the beginning of
   * the ramdisk.
   */
  uint32_t data;
  /*
   * The length of the data in bytes
   */
  uint32_t length;
};

void initrd_init(void);

#endif /* !__FS_INITRD_H__ */
