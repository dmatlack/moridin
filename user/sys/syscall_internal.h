#ifndef SYSCALL_INTERNAL_H
#define SYSCALL_INTERNAL_H

#define SYS_WRITE  0
#define SYS_GETPID 1
#define SYS_FORK   2

int __syscall(int system_call, void *arg1, void *arg2, void *arg3, void *arg4);

#define SYSCALL0(s) \
  __syscall(s, 0, 0, 0, 0)
#define SYSCALL1(s, a1) \
  __syscall(s, (void *) a1, 0, 0, 0)
#define SYSCALL2(s, a1, a2) \
  __syscall(s, (void *) a1, (void *) a2, 0, 0)
#define SYSCALL3(s, a1, a2, a3) \
  __syscall(s, (void *) a1, (void *) a2, (void *) a3, 0)
#define SYSCALL4(s, a1, a2, a3, a4) \
  __syscall(s, (void *) a1, (void *) a2, (void *) a3, (void *) a4)

#endif /* !SYSCALL_INTERNAL_H */
