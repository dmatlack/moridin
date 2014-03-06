/**
 * @file x86/cpu.h
 *
 */
#ifndef __X86_CPU_H__
#define __X86_CPU_H__

#include <stdint.h>

void enable_paging(void);
void disable_paging(void);
void enable_protected_mode(void);
void enable_real_mode(void);
void disable_fpu(void);
void enable_global_pages(void);
void enable_write_protect(void);

/**
 * @brief esp0 is a 4-byte file in the Task State Segment (TSS). It
 * identifies a region of memory to use as a stack in the event of a
 * priveldge level change (3 -> 0), which are usually caused by system 
 * calls and interrupts.
 */
void set_esp0(uint32_t);

// FIXME: move this somewhere else?
void jump_to_userspace(void);
// FIXME: move this somewhere else?
void jump_stacks(uint32_t stack, void (*func)(void *), void *arg);

#endif /* !__X86_CPU_H__ */
