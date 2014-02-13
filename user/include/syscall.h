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

// TODO: errno
//TODO: char **environ

#if 0
void _exit(void);
int close(int file);
int execve(char *name, char **argv, char **env);
int fork(void);
int fstat(int file, struct stat *st);
int getpid(void);
int isatty(int file);
int kill(int pid, int sig);
int lseek(int file, int ptr, int dir);
int open(const char *name, int flags, int mode);
int read(int file, char *ptr, int len);
size_t sbrk(int incr);
int stat(char *name, struct stat *st);
int times(struct tms *buf);
int unlink(char *name);
int wait(int *status);
int write(int file, char *ptr, int len);
#endif

int __syscall(int syscall, void *arg);

#endif /* !SYSCALL_H */
