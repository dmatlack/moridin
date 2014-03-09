/**
 * @file kernel/fork.c
 *
 * @brief Implementation of the fork system call.
 *
 */
#include <kernel/syscall.h>
#include <kernel/proc.h>
#include <kernel/kmalloc.h>
#include <kernel/sched.h>

#include <mm/vm.h>

#include <types.h>
#include <errno.h>

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

  /*
   * Copy the current thread's registers struct (the registers that will be
   * restored to this thread when it returns to userland) into the new 
   * thread's registers struct. This will ensure that when we finally context
   * switch to the new thread it will return from fork.
   */
  memcpy(&new_thread->regs, &CURRENT_THREAD->regs, sizeof(struct registers));

  /*
   * We set the register used to pass system call return values to be 0
   * because the child returns 0 from fork().
   */
  __set_syscall_return_reg(&new_thread->regs, 0);

  schedule(new_thread);

  /*
   * Return to the parent
   */
  return new_process->pid;

sys_fork_fail:
  if (new_thread) free_thread_struct(new_thread);
  if (new_process) free_process_struct(new_process);
  return error > 0 ? -1 * error : error;
}
