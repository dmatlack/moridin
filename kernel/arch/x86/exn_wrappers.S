###
# @file x86/exn_handlers.S
#
# @brief Exception handler handler functions. This file generates exception
# handlers for all the x86 exceptions. Each handler pushes state onto the
# stack and then calls a common exception handler which will then handle
# exceptions.
#
###

#
# Function used to return from an exception
#
.extern restore_registers

#
# Exception handling jump table
#
.extern exn_table

#
# @brief This macro generates a function for a given exception
#
# @param vector The exception vector (e.g 13 for GPF)
# @param has_error_code non-zero if the exception pushes an error code
# onto the stack, 0 otherwise.
# @param handler The handler to call after the stack has been set up
#
.macro exn_handler_macro vector, has_error_code, handler

# WARNING
#   If you change the name of the generated functions, you must also change
#   the prototypes in inc/x86/exn.h !!!!!!!!!!!!!!
.global exn_\vector
exn_\vector: 
	#
	# Note that we don't generate the iret stack even though it is part
	# of the registers struct. This is because the iret stack is provided 
	# already by the machine when the exception occurs.
	#

	#
	# There is an error code on the stack. We are trying to build up the
	# registers struct here so we need to move the error code somewhere else.
	# At this point, above (esp) is the iret stack. Below (esp) we will begin
	# pushing the rest of the registers stack. And then we will push the address
	# of the registers stack to pass as an argument to the exception handler.
	#
	push %eax               # we need to use a register to move data around so save the old value of eax on the stack
	addl $4, %esp           # move esp back up to the error code on the stack
.if \has_error_code
	movl (%esp), %eax       # save the error code in eax
.else
	movl $0, %eax           # if there is no error code, use 0
.endif
	movl %eax, -0x38(%esp)  # save the error code after where we push the registers
	subl $4, %esp           # restore esp so it points to the old value of eax
	popl %eax               # restore eax
.if \has_error_code
	addl $4, %esp           # finally move esp up again so the next push overwrites the error code
.endif

	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs

	pushl %eax;
	pushl %ecx;
	pushl %edx;
	pushl %ebx;
	pushl %ebp;
	pushl %esi;
	pushl %edi;

	movl %cr2, %eax
	pushl %eax
	movl %cr3, %eax
	pushl %eax

	movl %esp, %eax       # push the adddress of the registers struct on the stack
	pushl %eax

	subl $4, %esp         # jump over the error code on the stack

	pushl $\vector        # push the exception vector onto the stack

	#
	# The handler responsible for this exception is calculated using a jump table
	#
	movl $\vector, %eax
	movl $exn_table, %ecx
	movl (%ecx, %eax, 0x4), %ecx
	call *%ecx

	popl %eax             # pop off the vector
	popl %eax             # pop off the error code

	#
	# esp now points to the address of the registers struct
	#
	call restore_registers

.endm

#
# Now generate all the exception handlers.
#
exn_handler_macro  0, 0
exn_handler_macro  1, 0
exn_handler_macro  2, 0
exn_handler_macro  3, 0
exn_handler_macro  4, 0
exn_handler_macro  5, 0
exn_handler_macro  6, 0
exn_handler_macro  7, 0
exn_handler_macro  8, 1
exn_handler_macro  9, 0
exn_handler_macro 10, 1
exn_handler_macro 11, 1
exn_handler_macro 12, 1
exn_handler_macro 13, 1
exn_handler_macro 14, 1
exn_handler_macro 15, 0
exn_handler_macro 16, 0
exn_handler_macro 17, 1
exn_handler_macro 18, 0
exn_handler_macro 19, 0
