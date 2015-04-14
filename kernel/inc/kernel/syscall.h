/**
 * @file kernel/syscall.h
 */
#ifndef __KERNEL_SYSCALL_H__
#define __KERNEL_SYSCALL_H__

#define SYS_WRITE		0
#define SYS_GETPID		1
#define SYS_FORK		2
#define SYS_YIELD		3
#define SYS_EXIT		4
#define SYS_MAX                 5

#ifndef ASSEMBLER

#include <types.h>

extern void *syscall_table[];

int sys_write(int fd, char *ptr, int len);
int sys_getpid(void);
pid_t sys_fork(void);
int sys_yield(void);
void sys_exit(int status);

void bad_syscall(int syscall);

#endif

#endif /* !__KERNEL_SYSCALL_H__ */
