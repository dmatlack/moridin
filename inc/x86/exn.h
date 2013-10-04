/**
 * @file exn.h
 *
 * @brief Defines for x86 Exceptions.
 *
 * Faults: An exception that can generally be corrected and the program can
 *    be restarted with no loss of continuity. When the fault occurs, the
 *    machine resets its state to the state PRIOR to the instruction that
 *    caused the fault. Thus the "return address" of the exception handler 
 *    points to the instruction that caused the fault.
 *
 * Traps: An exception that is reported immediately following the execution 
 *    of the trapping instruction. The return address of the trap handler
 *    points to the instruction following the trapping instruction.
 *
 * Aborts: An abort is an exception that does not always report the location
 *    of the instruction causing the exception and does not allow the 
 *    program to continue execution after the handler completes.
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

void x86_exception_handle_all(
                  /* exception vector */
                  uint32_t vector,
                  /* control registers */
                  uint32_t cr2, uint32_t cr3,
                  /* segment selectors */
                  uint32_t ds, uint32_t es, uint32_t fs, uint32_t gs,
                  /* pusha registers */
                  unsigned edi,    unsigned esi, unsigned ebp,
                  unsigned ignore, unsigned ebx, unsigned edx,
                  unsigned ecx,    unsigned eax, 
                  /* error code for exception  */
                  unsigned error_code,
                  /* machine generated state information */
                  unsigned eip,    unsigned cs,  unsigned eflags,
                  unsigned esp,    unsigned ss);

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
