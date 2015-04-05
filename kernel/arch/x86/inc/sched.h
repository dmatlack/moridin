/**
 *     @file arch/x86/sched.h
 */
#ifndef __ARCH_X86_SCHED_H__
#define __ARCH_X86_SCHED_H__

#include <kernel/debug.h>

void __context_switch(void **save_addr, void *restore_addr);

static inline void context_switch(struct thread *to)
{
	__context_switch(&CURRENT_THREAD->context, to->context);
}

#endif /* !__ARCH_X86_SCHED_H__ */
