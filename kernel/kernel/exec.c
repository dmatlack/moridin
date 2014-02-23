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

struct proc_struct init_proc;

#include <arch/cpu.h> // iret_to_userspace

/**
 * @brief Load and initialize the first process that will run.
 */
void run_first_proc(char *execpath, int argc, char **argv) {
  struct vfs_file *file;
  struct thread_struct *thread;
  int ret;

  ret = proc_fork(NULL, &init_proc);
  ASSERT_EQUALS(0, ret);

  thread = list_head(&init_proc.threads);

  file = vfs_file_get(execpath);
  ASSERT_NOT_NULL(file);

  ret = exec_file_init(&init_proc.exec, file);
  ASSERT_EQUALS(0, ret);

  ret = vm_space_init(&init_proc.space);
  ASSERT_EQUALS(0, ret);
  __vm_space_switch(init_proc.space.object);

  /*
   * Load the executable into memory
   */
  ret = load(&init_proc.exec, &init_proc.space);
  ASSERT_EQUALS(0, ret);

  /*
   * Create a runtime stack for the process.
   */
  ret = create_user_stack(thread, argc, argv);
  ASSERT_EQUALS(0, ret);

  iret_to_userspace(thread->kstack_hi, (size_t) thread->proc->space.object,
                    thread->proc->exec.entry, thread->ustack_entry);
}
