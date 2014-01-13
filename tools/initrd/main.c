// BEGIN KERNEL INCLUDES
#include <initrd.h>
// END KERNEL INCLUDES

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define INITRD_FNAME "initrd"

#define fail(_fmt, ...) \
  do { \
    printf(_fmt"\n", ##__VA_ARGS__); \
    exit(1); \
  } while (0)

#define WRITE(_buf, _size, _num, _stream) \
  do { \
    int __ret; \
    if (_num != (__ret = fwrite((void *) _buf, _size, _num, _stream))) { \
      fclose(_stream); \
      fail("Write failed: %s:%d (returned %d)", __FILE__, __LINE__, __ret); \
    } \
  } while (0)

int main(int argc, char **argv) {
  FILE *f, *rdisk;
  unsigned nfiles = argc - 1;
  unsigned i;
  struct initrd_hdr hdr;
  struct initrd_file rfile;
  unsigned data_offset = 0;

  rdisk = fopen(INITRD_FNAME, "wb");
  if (!rdisk) {
    fail("Couldn't create "INITRD_FNAME);
  }

  hdr.magic = INITRD_MAGIC;
  hdr.nfiles = nfiles;

  WRITE(&hdr, sizeof(struct initrd_hdr), 1, rdisk);

  data_offset = sizeof(struct initrd_hdr) + nfiles * sizeof(struct initrd_file);

  /*
   * First write out all the files headers to the ramdisk
   */
  for (i = 1; i <= nfiles; i++) {
    char *fname = argv[i], *cp;
    FILE *f = fopen(fname, "r");
    if (!f) {
      fail("Couldn't open file");
    }

    /*
     * Just grab the end of the file path for the initrd filename
     */
    cp = strlen(fname) + fname;
    while (cp > fname) {
      if (*cp == '/') { cp++; break; }
      cp--;
    }

    printf("Adding file %s\n", cp);
    
    memcpy(rfile.name, cp, strlen(cp) + 1);
    fseek(f, 0, SEEK_END);
    rfile.length = ftell(f);
    rfile.data = data_offset;

    fclose(f);
    WRITE(&rfile, sizeof(struct initrd_file), 1, rdisk);

    data_offset += rfile.length;
  }

  /*
   * Then write the actual file data to the ramdisk
   */
   //
   //
   // TODO
   //
   //
}
