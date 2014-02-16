/**
 * @file kernel/fork.c
 *
 * @brief Implementation of the fork system call.
 *
 * @author David Matlack
 */
#include <kernel/syscall.h>
#include <kernel/proc.h>
#include <kernel/kmalloc.h>

#include <errno.h>

int sys_fork(void) {
  struct proc_struct *child;
  int ret;

  child = kmalloc(sizeof(struct proc_struct));
  if (NULL == child) {
    return ENOMEM;
  }

  ret = proc_fork(CURRENT_PROC, child);
  if (ret) {
    goto free_child_and_ret;
  }

free_child_and_ret:
  kfree(child, sizeof(struct proc_struct));
  return ret;
}
