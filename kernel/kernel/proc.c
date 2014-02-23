/**
 * @file kernel/proc.c
 *
 * @brief Process and Thread management.
 *
 * @author David Matlack
 */
#include <kernel/proc.h>
#include <kernel/atomic.h>
#include <kernel/loader.h>
#include <kernel/kmalloc.h>

#include <mm/vm.h>

#include <string.h>
#include <errno.h>
#include <debug.h>
#include <assert.h>
#include <list.h>

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

  thread->kstack_hi = (size_t) (thread->kstack + THREAD_KSTACK_SIZE);
  thread->kstack_lo = (size_t) (thread->kstack);

  ASSERT_EQUALS(thread->kstack_lo, thread);
  ASSERT(IS_PAGE_ALIGNED(thread->kstack_lo));

  list_elem_init(thread, thread_link);
}

int proc_add_thread(struct proc_struct *proc) {
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

int proc_new(struct proc_struct *proc) {
  proc_init(proc);
  return proc_add_thread(proc);
}

void destroy_thread(struct thread_struct *thread) {
  (void) thread; panic("%s: UNIMPLEMENTED", __func__);
}

void destroy_proc(struct proc_struct *proc) {
  (void) proc; panic("%s: UNIMPLEMENTED", __func__);
}

int thread_fork(struct thread_struct *parent, struct thread_struct *child) {
  memcpy(child->kstack, parent->kstack, THREAD_KSTACK_SIZE);

  child->ustack_start = parent->ustack_start;
  child->ustack_size = parent->ustack_size;

  return 0;
}

int proc_fork(struct proc_struct *parent, struct proc_struct *child) {
  int ret;

  ret = proc_new(child);
  if (ret) {
    return ret;
  }

  if (parent) {
    child->parent = parent;
    list_insert_tail(&parent->children, child, sibling_link);

    child->exec = exec_file_copy(parent->exec);

    child->arg_start = parent->arg_start;
    child->arg_size = parent->arg_size;
    child->argc_addr = parent->argc_addr;
    child->argv_addr = parent->argv_addr;

    //TODO child->space

    ret = thread_fork(list_head(&parent->threads), list_head(&child->threads));
    if (ret) {
      destroy_proc(child);
      return ret;
    }
  }

  return 0;
}
