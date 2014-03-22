/**
 * @file arch/x86/fork.h
 */
#ifndef __ARCH_X86_FORK_H__
#define __ARCH_X86_FORK_H__

#include <kernel/proc.h>
#include <stddef.h>

extern void __fork_context(void **save_addr, void *restore_addr);

/**
 * @brief This function returns twice. Once as the new thread (the argument to
 * this function) and once as the original caller.
 *
 * Essentially this function copies the kernel stack context of the currently
 * running thread (process) to the new thread. It also sets up the new thread
 * for a context switch. Thus this thread can added to the scheduler after
 * this call. The new thread can be context-switched-to after this function
 * returns. After the new thread is context-switched-to it will return from
 * this function.
 */
void fork_context(struct thread *new_thread) {
  TRACE("new_thread=%p", new_thread);
  __fork_context(&new_thread->context, CURRENT_THREAD->context);
}

void copy_context(void **addr) {
  struct thread *new_thread = container_of(addr, struct thread, context);

  TRACE("new_thread=%p", new_thread);

  /*
   * Copy the entire kernel stack from the current thread to the new thread.
   */
  memcpy((void *) _KSTACK_START(new_thread), (void *) _KSTACK_START(CURRENT_THREAD), KSTACK_SIZE);

  new_thread->context = (void *) (((size_t) (*addr)) - (size_t) CURRENT_THREAD + (size_t) new_thread);
}

#endif /* !__ARCH_X86_FORK_H__ */
