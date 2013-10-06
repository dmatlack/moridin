/**
 * @file kernel/exn.c
 *
 * @brief Exception handling in the kernel.
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>

#include <x86/exn.h>

/**
 * @brief The kernel's global exception handler for the x86 architecture.
 */
void kernel_x86_exn_handler(struct x86_exn_args *exn) {

  x86_exn_print(exn->vector, dprintf);
  x86_exn_print(exn->vector, kprintf);

  while (1);
}
