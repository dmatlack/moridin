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
# @brief This macro generates a function for a given exception
#
# @param vector The exception vector (e.g 13 for GPF)
# @param has_error_code non-zero if the exception pushes an error code
# onto the stack, 0 otherwise.
#
.macro exn_handler_macro vector, has_error_code 

# WARNING
#   If you change the name of the generated functions, you must also change
#   the prototypes in inc/x86/exn.h !!!!!!!!!!!!!!
.global exn_\vector
exn_\vector: 
  #
  # Note that we don't generate the iret stack even though it is passed
  # to the global handler. This is because the iret stack is provided 
  # already by the machine when the exception occurs.
  #

  .if \has_error_code   # put an error code onto the stack for exceptions
    nop                 # that don't already provide an error code. this
  .else                 # way we can have a common exception handler for
    pushl $0            # all exceptions
  .endif

  pusha                 # push all general purpose registers

  pushl %gs             # push all data segment registers
  pushl %fs
  pushl %es
  pushl %ds

  movl %cr4, %eax       # push the control registers onto the stack
  pushl %eax  
  movl %cr3, %eax
  pushl %eax
  movl %cr2, %eax
  pushl %eax
  movl %cr0, %eax
  pushl %eax

  pushl $\vector        # push the exception vector onto the stack

  call x86_exn_handle_all

  addl $4, %esp         # pop the vector off the stack
  
  addl $4, %esp         # pop the contol registers off the stack
  addl $4, %esp
  addl $4, %esp
  addl $4, %esp
  
  popl %ds              # pop the data segment registers off the stack
  popl %es
  popl %fs
  popl %gs

  popa                  # popa the pusha

  addl $4, %esp         # pop off the error code
  
  iret                  # and return to user land!
.endm

#
# Now generate all the exception handlers. Yes this is bad. I should be
# punished.
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
