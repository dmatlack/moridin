/**
 * @file sys/syscall.c
 */
#include <syscall.h>
#include "syscall_internal.h"

int write(int fd, char *ptr, int len) {
  return __syscall(SYS_WRITE, (void *) fd, (void *) ptr, (void *) len, 0);
}