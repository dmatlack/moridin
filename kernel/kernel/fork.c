/**
 * @file kernel/fork.c
 *
 * @brief Implementation of the fork system call.
 *
 */
#include <kernel/syscall.h>
#include <kernel/proc.h>
#include <kernel/kmalloc.h>

#include <errno.h>

int sys_fork(void) {
  struct process *child;
  int ret;

  child = kmalloc(sizeof(struct process));
  if (NULL == child) {
    return ENOMEM;
  }

  ret = proc_fork(CURRENT_PROC, child);
  if (ret) {
    goto free_child_and_ret;
  }

free_child_and_ret:
  kfree(child, sizeof(struct process));
  return ret;
}
