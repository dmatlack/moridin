/**
 * @file kernel/exec.h
 */
#ifndef __KERNEL_EXEC_H__
#define __KERNEL_EXEC_H__

extern struct thread  init_thread;
extern struct process init_proc;

void run_init(char *execpath, int argc, char **argv);

#endif /* !__KERNEL_EXEC_H__ */
