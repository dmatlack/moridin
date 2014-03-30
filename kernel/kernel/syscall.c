/**
 * @brief kernel/syscall.c
 *
 */
#include <kernel/syscall.h>
#include <kernel/kprintf.h>
#include <kernel/proc.h>
#include <kernel/sched.h>

#include <kernel/debug.h>

#define SYS_WRITE     0
#define SYS_GETPID    1
#define SYS_FORK      2
#define SYS_YIELD     3

void *syscall_table[] =
{
  [SYS_WRITE]  = (void *) sys_write,
  [SYS_GETPID] = (void *) sys_getpid,
  [SYS_FORK]   = (void *) sys_fork,
  [SYS_YIELD]  = (void *) sys_yield,
};

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
  sched_switch();
  return 0;
}
