/**
 * @file kernel/fork.c
 *
 * @brief Implementation of the fork system call.
 *
 */
#include <kernel/syscall.h>
#include <kernel/proc.h>
#include <kernel/sched.h>

#include <mm/vm.h>
#include <mm/kmalloc.h>

#include <arch/fork.h>

#include <types.h>
#include <errno.h>

/*
 * __next_pid is the next process id to give out. It starts at 2 because
 * 1 is reserved for init (see kernel/init.c).
 */
int __next_pid = 2;

int next_pid(void) {
  return atomic_add(&__next_pid, 1);
}

pid_t sys_fork(void) {
  struct thread *new_thread = NULL;
  struct process *new_process = NULL;
  int error;

  TRACE();

  if (num_threads(CURRENT_PROC) > 1) {
    /*
     * For POSIX specification on multithreaded fork see:
     * http://pubs.opengroup.org/onlinepubs/000095399/functions/fork.html
     */
    DEBUG("Multithreaded fork() not supported at the moment.");
    return -1;
  }

  error = ENOMEM;

  new_process = new_process_struct();
  if (!new_process) {
    goto sys_fork_fail;
  }

  new_thread = new_thread_struct();
  if (!new_thread) {
    goto sys_fork_fail;
  }

  error = vm_space_fork(&new_process->space, &CURRENT_PROC->space);
  if (error) {
    goto sys_fork_fail;
  }

  add_thread(new_process, new_thread);
  add_child_process(CURRENT_PROC, new_process);

  fork_context(new_thread);

  sched_make_runnable(new_thread);
  return new_process->pid;

sys_fork_fail:
  if (new_thread) free_thread_struct(new_thread);
  if (new_process) free_process_struct(new_process);
  return error > 0 ? -1 * error : error;
}
