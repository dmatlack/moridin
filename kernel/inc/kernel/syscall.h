/**
 * @file kernel/syscall.h
 */
#ifndef __KERNEL_SYSCALL_H__
#define __KERNEL_SYSCALL_H__

#include <types.h>

#define SYS_WRITE     0
#define SYS_GETPID    1
#define SYS_FORK      2
#define SYS_YIELD     3

int sys_write(int fd, char *ptr, int len);
int sys_getpid(void);
pid_t sys_fork(void);
int sys_yield(void);

#endif /* !__KERNEL_SYSCALL_H__ */
