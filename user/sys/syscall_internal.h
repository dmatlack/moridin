#ifndef SYSCALL_INTERNAL_H
#define SYSCALL_INTERNAL_H

#define SYS_WRITE 0

int __syscall(int system_call, void *arg1, void *arg2, void *arg3, void *arg4);

#endif /* !SYSCALL_INTERNAL_H */
