/**
 * @brief kernel/syscall.c
 *
 */
#include <kernel/syscall.h>
#include <kernel/kprintf.h>
#include <kernel/proc.h>
#include <kernel/sched.h>

#include <kernel/debug.h>

void *syscall_table[] =
{
	[SYS_WRITE]	= (void *) sys_write,
	[SYS_GETPID]	= (void *) sys_getpid,
	[SYS_FORK]	= (void *) sys_fork,
	[SYS_YIELD]	= (void *) sys_yield,
	[SYS_EXIT]	= (void *) sys_exit,
	[SYS_WAIT]	= (void *) sys_wait,
};

int sys_write(int fd, char *ptr, int len)
{
	int i;

	TRACE("fd=%d, ptr=%p, len=%d", fd, ptr, len);

	for (i = 0; i < len; i++) {
		log("%c", ptr[i]);
	}

	return 0;
}

int sys_getpid(void)
{
	TRACE();
	return CURRENT_THREAD->proc->pid;
}

int sys_yield(void)
{
	TRACE();
	sched_switch();
	return 0;
}

void bad_syscall(int syscall) {
	panic("Unknown syscall: %d\n", syscall);
}
