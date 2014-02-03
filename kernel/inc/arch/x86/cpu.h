/**
 * @file x86/cpu.h
 *
 * @author David Matlack
 */
#ifndef __X86_CPU_H__
#define __X86_CPU_H__

#include <stdint.h>

void x86_enable_paging(void);
void x86_disable_paging(void);
void x86_enable_protected_mode(void);
void x86_enable_real_mode(void);
void x86_disable_fpu(void);
void x86_enable_global_pages(void);
void x86_enable_write_protect(void);

/**
 * @brief esp0 is a 4-byte file in the Task State Segment (TSS). It
 * identifies a region of memory to use as a stack in the event of a
 * priveldge level change (3 -> 0), which are usually caused by system 
 * calls and interrupts.
 */
void set_esp0(uint32_t);

#endif /* !__X86_CPU_H__ */
