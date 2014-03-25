/**
 * @brief kernel/syscall.c
 *
 */
#include <kernel/syscall.h>
#include <kernel/kprintf.h>
#include <kernel/proc.h>
#include <kernel/sched.h>

#include <kernel/debug.h>

extern void **syscall_table;

int sys_write(int fd, char *ptr, int len) {
  int i;

  TRACE("fd=%d, ptr=%p, len=%d", fd, ptr, len);

  for (i = 0; i < len; i++) {
    kprintf("%c", ptr[i]); 
  }

  return 0;
}

int sys_getpid(void) {
  TRACE();
  return CURRENT_THREAD->proc->pid;
}

int sys_yield(void) {
  TRACE();
  //sched_next();
  return 0;
}
