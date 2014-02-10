/**
 * @brief kernel/syscall.c
 *
 * @author David Matlack
 */
#include <kernel/syscall.h>
#include <kernel/kprintf.h>

#include <debug.h>

int syscall(int s, void *arg) {
  TRACE("s=%d, arg=%p", s, arg);

  kprintf("%s", (char *) arg);
  return 42;
}
