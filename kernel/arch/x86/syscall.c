/**
 * @file arch/x86/syscall.c
 */
#include <kernel/syscall.h>

void *syscall_table[] =
{
  [SYS_WRITE]  = (void *) sys_write,
  [SYS_GETPID] = (void *) sys_getpid,
  [SYS_FORK]   = (void *) sys_fork,
  [SYS_YIELD]  = (void *) sys_yield,
};
