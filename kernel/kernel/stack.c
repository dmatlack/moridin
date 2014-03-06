/**
 * @file kernel/stack.c
 *
 * @brief Utilities for creating the userspace runtime stack.
 *
 */
#include <kernel/stack.h>
#include <kernel/proc.h>
#include <kernel/config.h>
#include <arch/reg.h>

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
 * @brief Create a runtime stack for the current process.
 *
 * This function actually does 2 things:
 *  1. Allocate enough pages at the top of the user's address spcae to fit
 *     the program arguments (argc, argv), and then copies the program 
 *     arguments to those pages.
 *  2. Allocate some number of pages for the runtime stack.
 *
 * @return
 *    0 on success
 */
int create_process_stack(int argc, char **argv) {
  unsigned long stack_start, stack_length;
  unsigned long arg_start, arg_length;
  char **stack_argv;
  char *stack_chars;
  unsigned long error;
  unsigned long sp;
  int i;

  /*
   * The very top of the stack is pages reserved for the program's
   * arguments.
   */
  arg_length = PAGE_ALIGN_UP(arg_size(argc, argv));
  arg_start = CONFIG_USER_VIRTUAL_TOP - arg_length;

  /*
   * After the program arguments, there's the runtime stack.
   */
  stack_length  = PAGE_SIZE;
  stack_start = arg_start - stack_length;

  error = vm_mmap(stack_start, stack_length + arg_length, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, NULL, 0);
  if (error % PAGE_SIZE) {
    return error % PAGE_SIZE;
  }

  /*
   * The location of the argument array on the stack
   */
  stack_argv = (char **) arg_start;

  /*
   * The location of the actually character arrays on the stack.
   */
  stack_chars = (char *) (arg_start + ((argc+1) * sizeof(char *)));

  for (i = 0; i < argc; i++) {
    char *from = argv[i];
    char *to = stack_chars;
    int len_with_null = strlen(from) + 1;

    memcpy(to, from, len_with_null);
    stack_argv[i] = to;

    stack_chars += len_with_null;
  }
  stack_argv[argc] = NULL;

  /*
   * Set the stack pointer to be the top of the runtime stack.
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
   */
  sp = stack_start + stack_length;

  /*
   * argv
   */
  sp -= sizeof(char **);
  *((char ***) sp) = stack_argv; // not 'argv' because that address is in kernel memory

  /*
   * argc
   */
  sp -= sizeof(int);
  *((int *) sp) = argc;

  /*
   * ret
   *
   * Since we never actually return from _start, we write a NULL
   * as the return address.
   */
  sp -= sizeof(void *);
  *((void **) sp) = NULL;

  set_sp(sp);
  return 0;
}

