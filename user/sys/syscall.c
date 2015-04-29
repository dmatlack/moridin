/**
 * @file sys/syscall.c
 */
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>

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

int wait(int *status)
{
	return SYSCALL1(SYS_WAIT, status);
}

extern char _end;		/* Defined by the linker */
size_t sbrk(int incr)
{
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = &_end;
	}
	prev_heap_end = heap_end;
#if 0
	if (heap_end + incr > stack_ptr) {
		write (1, "Heap and stack collision\n", 25);
		abort ();
	}
#endif

	heap_end += incr;
	return (size_t) prev_heap_end;
}

int close(int fd)
{
	(void)fd;

	return -1;
}

int fstat(int fd, struct stat *st)
{
	(void) fd;

	st->st_mode = S_IFCHR;
	return 0;
}

int isatty(int fd)
{
	(void)fd;

	return 1;
}

int lseek(int fd, int ptr, int dir)
{
	(void)fd;
	(void)ptr;
	(void)dir;

	return 0;
}

int read(int fd, char *ptr, int len)
{
	(void)fd;
	(void)ptr;
	(void)len;

	return 0;
}
