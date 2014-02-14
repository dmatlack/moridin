/**
 * @brief kernel/syscall.c
 *
 * @author David Matlack
 */
#include <kernel/syscall.h>
#include <kernel/kprintf.h>

#include <debug.h>

void *syscall_table[] = { (void *) sys_write };

int sys_write(int fd, char *ptr, int len) {
  int i;

  TRACE("fd=%d, ptr=%p, len=%d", fd, ptr, len);

  for (i = 0; i < len; i++) {
    kprintf("%c", ptr[i]); 
  }

  return 0;
}
