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

#include <lib/string.h>

/**
 * @brief Determine the size of the argument stack. This should be large
 * enough to hold all the character arrays in argv as well as argv itself.
 */
static size_t arg_size(int argc, char **argv) {
  size_t argv_size, size;
  int i;
  
  /*
   * According to http://www.gnu.org/software/libc/manual/html_node/Program-Arguments.html,
   * "A null pointer always follows the last element: argv[argc] is this
   * null pointer." So we reserve space for argc + 1 char pointers.
   */
  argv_size = (argc + 1) * sizeof(char *);

  size = 0;
  for (i = 0; i < argc; i++) {
    /*
     * +1 for the null terminator
     */
    size += strlen(argv[argc]) + 1;
  }
  
  return size + argv_size;
}

/**
 * @brief Set up the user runtime stack.
 *
 *  +-------+ <- top of runtime stack
 *  | argv  |
 *  +-------+
 *  | argc  |
 *  +-------+
 *  | ret   |
 *  +-------+ <- esp
 *  |       |
 *     ...
 *
 * @warning It is assumed that argv is already fully copied to the stack
 * and that argv points to an address on the stack!
 *
 * @param argv The location of the argument array on the stack.
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
 * @brief Copy the program arguments onto a region of the program's runtime
 * stack.
 */
void setup_arg_stack(struct thread_struct *thread, int argc, char **argv) {
  char **stack_argv;
  char *stack_arg;
  int i;

  /*
   * The location of the argument array on the stack
   */
  stack_argv = (char **) thread->proc->arg_start;

  /*
   * The location of the actually character arrays on the stack.
   */
  stack_arg = (char *) (thread->proc->arg_start + ((argc+1) * sizeof(char *)));

  for (i = 0; i < argc; i++) {
    char *from = argv[i];
    char *to = stack_arg;
    int len_with_null = strlen(from) + 1;

    memcpy(to, from, len_with_null);
    stack_argv[i] = to;

    stack_arg += len_with_null;
  }

  stack_argv[argc] = NULL;
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

  /*
   * Map a region for the program arguments
   */
  thread->proc->arg_start = CONFIG_USER_VIRTUAL_TOP - PAGE_SIZE;
  thread->proc->arg_size  = PAGE_ALIGN_UP(arg_size(argc, argv));

  ret = vm_map(thread->proc->space,
               thread->proc->arg_start, thread->proc->arg_size,
               VM_U | VM_W);
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

  setup_arg_stack(thread, argc, argv);
  setup_runtime_stack(thread, argc, (char **) thread->proc->arg_start);

  return 0;
}
