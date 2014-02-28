/**
 * @file kernel/exec.c
 *
 * !!!
 *
 * Right now this file just contains the code to run the first process. This
 * code is very close to what exec actually does.
 *
 * !!!
 *
 * @author David Matlack
 */
#include <kernel/exec.h>
#include <kernel/loader.h>
#include <kernel/proc.h>
#include <kernel/stack.h>

#include <fs/vfs.h>

#include <assert.h>
#include <list.h>

struct process init_proc;

#include <arch/cpu.h> // iret_to_userspace, jump_stacks

struct exec_args {
  char * execpath;
  int    argc;
  char **argv;
};

void finish_run_first_proc(struct exec_args *args) {
  struct vfs_file *file;
  int ret;
  ASSERT_EQUALS(CURRENT_PROC, &init_proc);

  ret = vm_space_init(&CURRENT_PROC->space);
  ASSERT_EQUALS(0, ret);

  __vm_space_switch(CURRENT_PROC->space.object);

  file = vfs_file_get(args->execpath);
  ASSERT_NOT_NULL(file);

  ret = exec_file_init(&CURRENT_PROC->exec, file);
  ASSERT_EQUALS(0, ret);

  ret = load(&CURRENT_PROC->exec);
  ASSERT_EQUALS(0, ret);

  ret = create_user_stack(CURRENT_THREAD, args->argc, args->argv);
  ASSERT_EQUALS(0, ret);

  iret_to_userspace(CURRENT_THREAD->kstack_hi,
                    (size_t) CURRENT_PROC->space.object,
                    CURRENT_PROC->exec.entry,
                    CURRENT_THREAD->ustack_entry);
}

/**
 * @brief Load and initialize the first process that will run.
 */
void run_first_proc(char *execpath, int argc, char **argv) {
  struct exec_args args;
  size_t init_kernel_stack;
  int ret;

  ret = proc_fork(NULL, &init_proc);
  ASSERT_EQUALS(0, ret);

  args.execpath = execpath;
  args.argc = argc;
  args.argv = argv;

  init_kernel_stack = list_head(&init_proc.threads)->kstack_hi;

  /*
   * Jump off of this initial boot stack and onto init's kernel
   * stack. This will allow the startup routines to the "get the
   * current process" by using the esp trick.
   */
  jump_stacks(init_kernel_stack, (void(*)(void*)) finish_run_first_proc, &args);
}
