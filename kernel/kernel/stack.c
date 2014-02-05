/**
 * @file kernel/stack.c
 *
 * @brief Utilities for creating the userspace runtime stack.
 *
 * @author David Matlack
 */
#include <kernel/stack.h>
#include <kernel/proc.h>
#include <kernel/config.h>

#include <assert.h>

#include <mm/vm.h>
#include <mm/memory.h>

/**
 * @brief Set up the user runtime stack.
 *
 *  +-------+ <- top of stack
 *  | argv  |
 *  +-------+
 *  | argc  |
 *  +-------+
 *  | ret   |
 *  +-------+ <- esp
 *  |       |
 *     ...
 */
void setup_runtime_stack(struct thread_struct *thread, int argc, char **argv) {

  thread->esp = thread->ustack_start + thread->ustack_size;

  /*
   * argv
   */
  thread->esp -= sizeof(char **);
  thread->proc->argv_addr = thread->esp;
  *((char ***) thread->proc->argv_addr) = argv;

  /*
   * argc
   */
  thread->esp -= sizeof(int);
  thread->proc->argc_addr = thread->esp;
  *((int *) thread->proc->argc_addr) = argc;

  /*
   * ret
   *
   * Since we never actually return from _start, we write a NULL
   * as the return address.
   */
  thread->esp -= sizeof(void *);
  *((void **) thread->esp) = NULL;
}

/**
 * @brief Create the main user runtime stack for the thread.
 *
 * This function actually does 2 things:
 *  1. Allocate enough pages at the top of the user's address spcae to fit
 *     the program arguments (argc, argv), and then copies the program 
 *     arguments to those pages.
 *  2. Allocate some number of pages for the runtime stack.
 *
 *  This function populates the thread_struct fields with the location of
 *  argc, argv, and the runtime stack.
 *
 * @return
 *    0 on success
 */
int create_user_stack(struct thread_struct *thread, int argc, char **argv) {
  int ret;

  if (argc != 0) {
    //TODO
    panic("Implement me: %s(argc > 0)", __func__);
  }
  (void) argc; (void) argv;

  /*
   * Map a read-only region for the program arguments
   */
  thread->proc->arg_start = CONFIG_USER_VIRTUAL_TOP - PAGE_SIZE;
  thread->proc->arg_size  = PAGE_SIZE;

  ret = vm_map(thread->proc->space,
               thread->proc->arg_start, thread->proc->arg_size,
               VM_U | VM_R);
  if (ret) {
    return ret;
  }

  /*
   * Map a read-write region for the runtime stack
   */
  thread->ustack_size  = PAGE_SIZE;
  thread->ustack_start = thread->proc->arg_start - thread->ustack_size;

  ret = vm_map(thread->proc->space,
               thread->ustack_start, thread->ustack_size,
               VM_U | VM_W);
  if (ret) {
    vm_unmap(thread->proc->space, thread->proc->arg_start, thread->proc->arg_size);
    return ret;
  }

  /*
   * FIXME implement copying the argv array into the arg region of the stack
   * and then pass in the address into this function.
   */
  setup_runtime_stack(thread, 0xaa, (char **) 0xbb);

  return 0;
}

