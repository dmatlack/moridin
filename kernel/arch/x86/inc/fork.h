/**
 * @file arch/x86/fork.h
 */
#ifndef __ARCH_X86_FORK_H__
#define __ARCH_X86_FORK_H__

#include <arch/vm.h>

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
void fork_context(struct thread *new_thread);

/**
 * @brief This function is responsible for making to_pd map the same address
 * space as from_pd.
 *
 * If this function succeeds, the following will be true:
 *    1. Any virtual address that has a present mapping in from_pd will also
 *       have a mapping in to_pd.
 *    2. All kernel virtual addresses in from_pd will be mapped into to_pd by
 *       only copying the page directory entries.
 *    3. All user virtual addresses in from_pd will be mapped into to_pd by 
 *       allocating a new page table for to_pd.
 *    4. All mapped virtual addresses will map to the _same physical page_
 *       in both address spaces.
 *    5. All userspace mappings in to_pd and from_pd will be read-only, to
 *       support copy-on-write.
 *
 * @return 0 on success, non-0 on error
 */
int fork_address_space(struct entry_table *to_pd, struct entry_table *from_pd);

#endif /* !__ARCH_X86_FORK_H__ */
