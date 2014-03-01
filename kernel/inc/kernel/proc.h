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

//FIXME: get_esp() should probably be called something else
#ifdef ARCH_X86
#include <arch/reg.h>
#endif

#define CURRENT_THREAD \
  ((struct thread *) PAGE_ALIGN_DOWN(get_esp()))

#define CURRENT_PROC \
  ((CURRENT_THREAD)->proc)

#define THREAD_STRUCT_ALIGN PAGE_SIZE
#define THREAD_KSTACK_SIZE 2048
struct thread {
  /*
   * the kernel stack used by this thread. MUST BE PAGE-ALIGNED.
   */
  char  kstack[THREAD_KSTACK_SIZE];
  size_t kstack_hi;
  size_t kstack_lo;

  /*
   * the process this thread is a part of
   */
  struct process *proc;

  /*
   * each thread is part of a linked list of siblings
   */
  list_link(struct thread) thread_link;

  /*
   * The location of the user runtime stack.
   */
  size_t ustack_start;
  size_t ustack_size;
  size_t ustack_entry;

  int tid;
};

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

  /*
   * The file this process is executing
   */
  struct exec_file exec;

  /*
   * The region of memory where process arguments are stored. This is usually
   * some number of pages at the top of the stack.
   */
  size_t arg_start;
  size_t arg_size;

  /*
   * The location of each argument on the user runtime stack.
   */
  size_t argc_addr;
  size_t argv_addr;

  int next_tid;
  int pid;
};

int proc_fork(struct process *parent, struct process *child);

#endif /* !_KERNEL_PROC_H__ */
