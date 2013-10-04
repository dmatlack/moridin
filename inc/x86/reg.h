/**
 * @file x86/reg.h
 *
 * @brief Functions and datastructures for manipulating x86 registers.
 *
 * @author David Matlack
 */
#ifndef __X86_REG_H_
#define __X86_REG_H_

#include <stdint.h>

struct __attribute__ ((__packed__)) x86_pusha_stack {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t ignore;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
}

struct __attribute__ ((__packed__)) x86_iret_stack {
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t esp;
  uint32_t ss;
}

#endif /* __X86_REG_H_ */
