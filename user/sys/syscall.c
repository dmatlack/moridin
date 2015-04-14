/**
 * @file sys/syscall.c
 */
#include <syscall.h>
#include "syscall_internal.h"

int write(int fd, char *ptr, int len)
{
	return SYSCALL3(SYS_WRITE, fd, ptr, len);
}

int getpid(void)
{
	return SYSCALL0(SYS_GETPID);
}

int fork(void)
{
	return SYSCALL0(SYS_FORK);
}

int yield(void)
{
	return SYSCALL0(SYS_YIELD);
}

void exit(int status)
{
	SYSCALL1(SYS_EXIT, status);
}
