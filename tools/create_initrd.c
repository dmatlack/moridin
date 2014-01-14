/**
 * @file tools/create_initrc.c
 *
 * @brief A utility to create an initial ramdisk
 *
 * @author David Matlack
 */

// BEGIN KERNEL INCLUDES
#include "../kernel/inc/fs/initrd.h"
// END KERNEL INCLUDES

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define INITRD_FNAME "initrd"

#define USAGE \
  "Usage: ./create_initrc files ...\n" \
  "   -h,--help  For this help message.\n" \
  "\n" \
  "Pass the utility a list of 0 or more files and it will create a\n" \
  "that can be used as an initial ramdisk.\n"
 

#define fail(_fmt, ...) \
  do { \
    printf(_fmt"\n", ##__VA_ARGS__); \
    exit(1); \
  } while (0)

#define WRITE(_buf, _size, _stream) \
  do { \
    int __ret; \
    if (_size != (__ret = fwrite((void *) _buf, 1, _size, _stream))) { \
      fclose(_stream); \
      fail("Write failed: %s:%d (returned %d)", __FILE__, __LINE__, __ret); \
    } \
  } while (0)

void copy_file(FILE *to, FILE *from) {
#define BUFSIZE 128
  char buf[BUFSIZE];
  int bytes;
  int error;

  while (0 != (bytes = fread(buf, 1, BUFSIZE, from))) {
    if (bytes < 0) {
      fclose(from);
      fclose(to);
      fail("Failed fread() in %s: %d", __func__, bytes);
    }
    if (0 > (error = fwrite(buf, 1, bytes, to))) {
      fclose(from);
      fclose(to);
      fail("Failed fwrite() in %s: %d", __func__, error);
    }
  }
}

int main(int argc, char **argv) {
  FILE *f, *rdisk;
  unsigned nfiles = argc - 1;
  unsigned i;
  struct initrd_hdr hdr;
  struct initrd_file rfile;
  unsigned data_start, data_offset;

  if (argc > 1 &&
      (!strncmp(argv[1], "-h", 2) || !strncmp(argv[1], "--help", 5))) {
    printf(USAGE);
    exit(0);
  }

  rdisk = fopen(INITRD_FNAME, "wb");
  if (!rdisk) {
    fail("Couldn't create "INITRD_FNAME);
  }

  /*
   * Write the initrd filesystem header to the beginning of the ramdisk
   */
  hdr.magic = INITRD_MAGIC;
  hdr.nfiles = nfiles;
  WRITE(&hdr, sizeof(struct initrd_hdr), rdisk);

  data_start = sizeof(struct initrd_hdr) + nfiles * sizeof(struct initrd_file);
  data_offset = data_start;

  /*
   * First write out all the files headers to the ramdisk
   */
  for (i = 1; i <= nfiles; i++) {
    char *fname = argv[i], *cp;
    FILE *f = fopen(fname, "r");
    if (!f) {
      fail("Couldn't open file %s to write header.", fname);
    }

    /*
     * Just grab the end of the file path for the initrd filename
     */
    cp = strlen(fname) + fname;
    while (cp > fname) {
      if (*cp == '/') { cp++; break; }
      cp--;
    }

    memcpy(rfile.name, cp, strlen(cp) + 1);
    fseek(f, 0, SEEK_END);
    rfile.length = ftell(f);
    rfile.data = data_offset;
    fclose(f);

    printf("Adding file %s (length=0x%x, data=0x%x)\n",
           rfile.name, rfile.length, rfile.data);
    WRITE(&rfile, sizeof(struct initrd_file), rdisk);

    data_offset += rfile.length;
  }

  /*
   * Then write the actual file data to the ramdisk
   */
  for (i = 1; i <= nfiles; i++) {
    char *fname = argv[i], *cp;
    FILE *f = fopen(fname, "r");
    if (!f) {
      fail("Couldn't open file %s to write data.", fname);
    }

    copy_file(rdisk, f);
  }

  return 0;
}
