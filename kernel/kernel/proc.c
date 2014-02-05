/**
 * @file kernel/proc.c
 *
 * @brief Process and Thread management.
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <kernel/proc.h>
#include <kernel/atomic.h>

#include <mm/vm.h>
#include <string.h>

#include <errno.h>
#include <debug.h>
#include <assert.h>

static int next_pid = 0;

static int get_next_pid() {
  return atomic_add(&next_pid, 1);
}

void proc_init(struct proc_struct *proc) {
  memset(proc, 0, sizeof(struct proc_struct));

  list_init(&proc->children);
  list_init(&proc->threads);
  list_elem_init(proc, sibling_link);
  
  proc->pid = get_next_pid();
}

void thread_init(struct thread_struct *thread) {
  memset(thread->kstack, 0, THREAD_KSTACK_SIZE);

  thread->kstack_hi = thread->kstack + THREAD_KSTACK_SIZE;
  thread->kstack_lo = thread->kstack;

  list_elem_init(thread, thread_link);
}

int add_thread(struct proc_struct *proc) {
  struct thread_struct *thread;

  thread = kmemalign(THREAD_STRUCT_ALIGN, sizeof(struct thread_struct));
  if (NULL == thread) {
    return ENOMEM;
  }

  thread_init(thread);
  thread->proc = proc;
  thread->tid = atomic_add(&proc->next_tid, 1);

  list_insert_tail(&proc->threads, thread, thread_link);
  
  return 0;
}

int new_proc(struct proc_struct *proc) {
  proc_init(proc);
  return add_thread(proc);
}
