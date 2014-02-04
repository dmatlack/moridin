/**
 * @file kernel.c
 *
 * Welcome. Have a look around.
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <kernel/kmalloc.h>
#include <kernel/irq.h>
#include <kernel/timer.h>
#include <kernel/exn.h>
#include <kernel/loader.h>
#include <kernel/proc.h>
#include <kernel/config.h>

#include <mm/memory.h>
#include <mm/pages.h>
#include <mm/vm.h>

#include <dev/vga.h>
#include <dev/serial.h>
#include <dev/pci.h>

#include <assert.h>
#include <debug.h>
#include <errno.h>

#include <fs/vfs.h>

extern struct vm_space *postboot_vm_space;
struct proc_struct init_proc;

/**
 * @brief Kernel initialization functions that need to run with interrupts
 * off.
 *
 * You probably don't want to mess with the order of these functions.
 */
void pre_irq_init(void) {
  debug_init();

  pages_init();
  vm_init();
  exn_init();
  irq_init();
  timer_init();
}

/**
 * @brief Kernel initialization functions that can, or should, run with
 * interrupts enabled.
 */
void post_irq_init(void) {
  pci_init();
}

//
// FIXME: hacked user runtime stack. Need to move to a different file,
// finish, and clean up.
//
int create_stack(struct proc_struct *proc, int argc, char **argv) {
  size_t args_start, args_size;
  size_t stack_start, stack_size;

  int *stack_argc;
  char ***stack_argv;
  int ret;

  if (argc != 0) {
    //TODO
    panic("Implement me: create_stack() (argc > 0)");
  }
  (void) argc; (void) argv;

  /*
   * Map a read-only region for the program arguments
   */
  args_start = CONFIG_USER_VIRTUAL_TOP - PAGE_SIZE;
  args_size = PAGE_SIZE;
  ret = vm_map(proc->space, args_start, args_size, VM_U | VM_R);
  if (ret) {
    return ret;
  }

  /*
   * Map a read-write region for the runtime stack
   */
  stack_size = PAGE_SIZE;
  stack_start = args_start - stack_size;
  ret = vm_map(proc->space, stack_start, stack_size, VM_U | VM_W);
  if (ret) {
    vm_unmap(proc->space, args_start, args_size);
    return ret;
  }

  stack_argc = (int *) args_start;
  stack_argv = (char ***) (args_start + sizeof(int));

  *(stack_argc) = 0;
  *(stack_argv) = NULL;

  list_head(&proc->threads)->ustack = stack_start + stack_size - sizeof(int);

  return 0;
}

//
// FIXME: hacked jump to user space. Need to separate this out into arch/
// and cleanup the code.
//
#include <arch/x86/reg.h>
#include <arch/x86/cpu.h>
#include <arch/x86/seg.h>
void jump_to_userspace(struct thread_struct *thread) {
  uint32_t page_dir;

  set_esp0((uint32_t) thread->kstack_hi);

  page_dir = (uint32_t) thread->proc->space->object;

  restore_registers(
      get_cr4(),                    // cr4
      page_dir,                     // cr3
      0,                            // cr2
      get_cr0(),                    // cr0

      0,                            // edi
      0,                            // esi
      0,                            // ebp
      0,                            // ignore
      0,                            // ebx
      0,                            // edx
      0,                            // ecx
      0,                            // eax

      SEGSEL_USER_DS,               // gs
      SEGSEL_USER_DS,               // fs
      SEGSEL_USER_DS,               // es
      SEGSEL_USER_DS,               // ds

      thread->proc->exec->entry,    // eip
      SEGSEL_USER_CS,               // cs
      get_eflags(),                 // eflags
      thread->ustack,               // esp
      SEGSEL_USER_DS                // ss
  );
}

/**
 * @brief Load and initialize the first process that will run.
 */
void run_first_proc(char *execpath) {
  struct vfs_file *file;
  int ret;

  ret = new_proc(&init_proc);
  ASSERT_EQUALS(0, ret);

  file = vfs_file_get(execpath);
  ASSERT(NULL != file);

  init_proc.exec = exec_file_get(file);
  ASSERT(NULL != init_proc.exec);

  init_proc.space = postboot_vm_space;

  /*
   * Load the executable into memory
   */
  ret = load(init_proc.exec, init_proc.space);
  ASSERT_EQUALS(ret, 0);

  /*
   * Create a runtime stack for the process.
   */
  ret = create_stack(&init_proc, 0, NULL);

  jump_to_userspace(list_head(&init_proc.threads));
}

/**
 * @brief This is main logical entry point for the kernel, not to be confused
 * with the actual entry point, _start. Also not to be confused the multiboot
 * entry point mb_entry. Ok so it's not the entry point, but it is an entry
 * point.
 */
void kernel_main() {
  pre_irq_init();
  enable_irqs();
  post_irq_init();

  run_first_proc((char*) "/init");
}
