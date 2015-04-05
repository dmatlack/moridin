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
 */
#ifndef __X86_EXN_H__
#define __X86_EXN_H__

#include <stdint.h>
#include <types.h>

#include <arch/reg.h>
#include <arch/idt.h>

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
	void (*handler)(void);
};

#define X86_NUM_EXCEPTIONS 20
extern struct x86_exn x86_exceptions[X86_NUM_EXCEPTIONS];

// WARNING
//  If you change this macro, you must also change the macro in
//  x86/exn_wrappers.S !!!!!!!!!!
#define EXN_HANDLER_NAME(vector) exn_##vector

#define EXN_HANDLER_PROTOTYPE(vector)   void EXN_HANDLER_NAME(vector)(void)

/*
 * Generate the exception handler prototypes
 */
EXN_HANDLER_PROTOTYPE(0);
EXN_HANDLER_PROTOTYPE(1);
EXN_HANDLER_PROTOTYPE(2);
EXN_HANDLER_PROTOTYPE(3);
EXN_HANDLER_PROTOTYPE(4);
EXN_HANDLER_PROTOTYPE(5);
EXN_HANDLER_PROTOTYPE(6);
EXN_HANDLER_PROTOTYPE(7);
EXN_HANDLER_PROTOTYPE(8);
EXN_HANDLER_PROTOTYPE(9);
EXN_HANDLER_PROTOTYPE(10);
EXN_HANDLER_PROTOTYPE(11);
EXN_HANDLER_PROTOTYPE(12);
EXN_HANDLER_PROTOTYPE(13);
EXN_HANDLER_PROTOTYPE(14);
EXN_HANDLER_PROTOTYPE(15);
EXN_HANDLER_PROTOTYPE(16);
EXN_HANDLER_PROTOTYPE(17);
EXN_HANDLER_PROTOTYPE(18);
EXN_HANDLER_PROTOTYPE(19);

void exn_panic(int vector, int error, struct registers *regs);

#endif /* __X86_EXN_H__ */
