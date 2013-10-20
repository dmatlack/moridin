/**
 * @file x86/cpu.h
 *
 * @author David Matlack
 */
#ifndef __X86_CPU_H__
#define __X86_CPU_H__

void x86_enable_paging(void);
void x86_disable_paging(void);
void x86_enable_protected_mode(void);
void x86_enable_real_mode(void);
void x86_disable_fpu(void);
void x86_enable_global_pages(void);

#endif /* !__X86_CPU_H__ */
