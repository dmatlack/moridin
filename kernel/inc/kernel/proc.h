/**
 * @file kernel/proc.h
 *
 * @author David Matlack
 */
#ifndef __KERNEL_PROC_H__
#define __KERNEL_PROC_H__

#include <list.h>
#include <mm/memory.h>


struct thread_struct;
struct proc_struct;

list_typedef(struct thread_struct) thread_list_t;
list_typedef(struct proc_struct) proc_list_t;

#define CURRENT_THREAD \
  ((struct thread_struct *) FLOOR(get_esp(), PAGE_SIZE))

#define THREAD_STRUCT_ALIGN PAGE_SIZE
#define THREAD_KSTACK_SIZE 2048
struct thread_struct {
  /*
   * the kernel stack used by this thread. MUST BE PAGE-ALIGNED.
   */
  char  kstack[THREAD_KSTACK_SIZE];
  char *kstack_hi;
  char *kstack_lo;

  /*
   * the process this thread is a part of
   */
  struct proc_struct *proc;

  /*
   * each thread_struct is part of a linked list of siblings
   */
  list_link(struct thread_struct) thread_link;

  /*
   * The location of the user runtime stack.
   */
  size_t ustack;

  int tid;
};

struct proc_struct {
  /*
   * the process family hierarchy: a pointer to your parent, and a list
   * of your children.
   */
  struct proc_struct *parent; 
  proc_list_t children;
  list_link(struct proc_struct) sibling_link;

  /*
   * All the threads in this process
   */
  thread_list_t threads;

  /*
   * The virtual address space shared by all threads running in the process.
   */
  struct vm_space *space;

  /*
   * The file this process is executing
   */
  struct exec_file *exec;

  int next_tid;
  int pid;
};

int new_proc(struct proc_struct *proc);

#endif /* !_KERNEL_PROC_H__ */
