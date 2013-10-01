/**
 * @file exn.h
 *
 * @brief Defines for x86 Exceptions.
 *
 * @author David Matlack
 */
#ifndef __X86_EXN_H__
#define __X86_EXN_H__

#include <stdint.h>
#include <types.h>

#define X86_TRAP  0x1
#define X86_FAULT 0x2
#define X86_ABORT 0x4

#define X86_IS_TRAP(type)   (type & X86_TRAP)
#define X86_IS_FAULT(type)  (type & X86_FAULT)
#define X86_IS_ABORT(type)  (type & X86_ABORT)

struct x86_exn {
  uint8_t vector;
  char    mnemonic[5];
  char    description[128];
  uint8_t type;
  char    cause[128];
  bool    has_error_code;
};

extern struct x86_exn x86_exceptions[20];


#endif /* __X86_EXN_H__ */
