/**
 * @file kernel/proc.h
 *
 */
#ifndef __KERNEL_PROC_H__
#define __KERNEL_PROC_H__

#include <kernel/loader.h>
#include <mm/memory.h>
#include <mm/vm.h>
#include <list.h>

struct thread;
struct process;

list_typedef(struct thread) thread_list_t;
list_typedef(struct process) proc_list_t;

#include <arch/reg.h>

#define CURRENT_THREAD \
  ((struct thread *) PAGE_ALIGN_DOWN(get_sp()))

#define CURRENT_PROC \
  ((CURRENT_THREAD)->proc)

#define KSTACK_SIZE 2048

#define _KSTACK_START(_thread)    ((unsigned long) (_thread)->kstack)
#define _KSTACK_END(_thread)      (_KSTACK_START(_thread) + KSTACK_SIZE)
#define _KSTACK_TOP(_thread)      (_KSTACK_END(_thread) - sizeof(void*))

#define KSTACK_START              _KSTACK_START(CURRENT_THREAD)
#define KSTACK_END                _KSTACK_END(CURRENT_THREAD)
#define KSTACK_TOP                _KSTACK_TOP(CURRENT_THREAD)

#define THREAD_STRUCT_ALIGN PAGE_SIZE
struct thread {
  /*
   * the kernel stack used by this thread. MUST BE PAGE-ALIGNED.
   */
  char  kstack[KSTACK_SIZE];

  /*
   * the process this thread is a part of
   */
  struct process *proc;

  /*
   * each thread is part of a linked list of siblings
   */
  list_link(struct thread) thread_link;

  struct registers regs;

  int tid;
} __attribute__((aligned (THREAD_STRUCT_ALIGN)));

struct process {
  /*
   * the process family hierarchy: a pointer to your parent, and a list
   * of your children.
   */
  struct process *parent; 
  proc_list_t children;
  list_link(struct process) sibling_link;

  /*
   * All the threads in this process
   */
  thread_list_t threads;

  /*
   * The virtual address space shared by all threads running in the process.
   */
  struct vm_space space;

  int next_tid;
  int pid;
};

#define num_threads(_proc) (list_size(&(_proc)->threads))
#define main_thread(_proc) (list_head(&(_proc)->threads))

int proc_fork(struct process *parent, struct process *child);

#endif /* !_KERNEL_PROC_H__ */
