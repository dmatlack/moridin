/**
 * @file sys/syscall.h
 *
 * System call interface and documentation designed to meet the requirements
 * of newlib:
 *    https://sourceware.org/newlib/libc.html#Syscalls
 *
 * @author David Matlack
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#if 0
void _exit(void);
int close(int fd);
int execve(char *name, char **argv, char **env);
int fstat(int fd, struct stat *st);
int isatty(int fd);
int kill(int pid, int sig);
int lseek(int fd, int ptr, int dir);
int open(const char *name, int flags, int mode);
int read(int fd, char *ptr, int len);
size_t sbrk(int incr);
int stat(char *name, struct stat *st);
int times(struct tms *buf);
int unlink(char *name);
int wait(int *status);
#endif

int write(int fd, char *ptr, int len);
int getpid(void);
int fork(void);

#endif /* !SYSCALL_H */
