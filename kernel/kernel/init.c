/**
 * @file kernel/exec.c
 *
 * @brief Bootstrap user space by setting up and running init.
 */
#include <kernel/init.h>
#include <mm/kmalloc.h>
#include <kernel/loader.h>
#include <kernel/proc.h>
#include <kernel/stack.h>
#include <kernel/test.h>

#include <arch/reg.h>
#include <arch/cpu.h>
#include <arch/vm.h>

#include <mm/vm.h>

#include <fs/vfs.h>

#include <assert.h>
#include <list.h>
#include <errno.h>

struct registers init_regs = INIT_REGS;

#define INIT_THREAD                                                  \
{                                                                    \
  .kstack      = { [0 ... KSTACK_SIZE-1] = (char) 0 },               \
  .proc        = &init_proc,                                         \
  .thread_link = INITIALIZED_LIST_LINK,                              \
  .tid         = 0,                                                  \
  .regs        = &init_regs,                                         \
}
#define INIT_PROCESS                                                 \
{                                                                    \
  .parent       = NULL,                                              \
  .children     = INITIALIZED_EMPTY_LIST,                            \
  .sibling_link = INITIALIZED_LIST_LINK,                             \
  .threads      = INITIALIZED_LIST(&init_thread),                    \
  .next_tid     = 1,                                                 \
  .pid          = 1,                                                 \
}

struct process init_proc    = INIT_PROCESS;
struct thread  init_thread  = INIT_THREAD;

struct {
  char * execpath;
  int    argc;
  char **argv;
} init_args;

static void setup_init_vm(void) {
  struct vm_space *space = &CURRENT_PROC->space;
  int error;

  error = vm_space_init(space);
  if (error) {
    panic("Failed set up virtual memory for init: %s", strerr(error));
  }

  /*
   * Switch the current address space (in the hardware MMU) to init's
   * address space.
   */
  swap_address_space(space->mmu);
}

static void load_init_binary(char *path) {
  struct vfs_file *file;
  int error;

  file = vfs_file_get(path);
  if (!file) {
    panic("Failed to get file: %s", path);
  }

  error = load_binary(file);
  if (error) {
    panic("Failed to load %s into memory: %s", path, strerr(error));
  }
}

void __run_init(void *ignore) {
  int error;

  (void) ignore;

  ASSERT_EQUALS(CURRENT_PROC, &init_proc);

  setup_init_vm();

  load_init_binary(init_args.execpath);

  error = create_process_stack(init_args.argc, init_args.argv);
  if (error) {
    panic("Couldn't initialize the runtime stack for init: %s", strerr(error));
  }

  vm_dump_maps(__log, &CURRENT_PROC->space);

  _TEST_KERNEL_();

  jump_to_userspace();
}

/**
 * @brief Load and initialize the first process that will run.
 */
void run_init(char *execpath, int argc, char **argv) {
  TRACE("execpath=%s, argc=%d, argv=%p", execpath, argc, argv);

  init_args.execpath = execpath;
  init_args.argc = argc;
  init_args.argv = argv;

  /*
   * Jump off of this initial boot stack and onto init's kernel
   * stack. This will allow the startup routines to the "get the
   * current process" by using the esp trick.
   */
  jump_stacks(_KSTACK_TOP(&init_thread), __run_init, NULL);
}
