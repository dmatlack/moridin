/**
 * @file kernel/syscall.h
 */
#ifndef __KERNEL_SYSCALL_H__
#define __KERNEL_SYSCALL_H__

#include <types.h>

int sys_write(int fd, char *ptr, int len);
int sys_getpid(void);
pid_t sys_fork(void);

#endif /* !__KERNEL_SYSCALL_H__ */
