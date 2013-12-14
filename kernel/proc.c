/**
 * @file kernel/proc.c
 *
 * @brief Process and Thread management.
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <kernel/proc.h>

#include <mm/vm.h>
#include <string.h>

#include <errno.h>
#include <debug.h>
#include <assert.h>

int __global_pid = 0;

static int __get_next_pid() {
  return ++__global_pid;
}

int proc_init(struct proc_struct *proc) {
  int ret;

  ASSERT(NULL != proc);

  list_init(&proc->children);
  proc->parent = NULL;

  ret = vm_address_space_create(&proc->vm);
  if (0 != ret) { return ret; }

  list_init(&proc->threads);

  proc->pid = __get_next_pid();
  proc->next_tid = 0;
  
  return 0;
}

int proc_create(struct proc_struct **procp) {
  ASSERT(NULL != procp);

  *procp = kmalloc(sizeof(struct proc_struct));
  if (NULL == *procp) {
    return ENOMEM;
  }

  return proc_init(*procp);
}

int thread_init(struct thread_struct *thread) {
  ASSERT(NULL != thread);

  memset(thread->kstack, 0, THREAD_KSTACK_SIZE);

  thread->kstack_lo = &thread->kstack[0];
  thread->kstack_hi = &thread->kstack[THREAD_KSTACK_SIZE - 1];

  /*
   * These will be set when we add this thread to a process
   */
  thread->tid = 0;
  thread->proc = NULL;
  list_elem_init(thread, sibling_link);

  return 0;
}

int thread_create(struct thread_struct **threadp) {
  ASSERT(NULL != threadp);

  *threadp = kmemalign(PAGE_SIZE, sizeof(struct thread_struct));
  if (NULL == *threadp) {
    return ENOMEM;
  }

  return thread_init(*threadp);
}
