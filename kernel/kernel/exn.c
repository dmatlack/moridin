/**
 * @file kernel/exn.c
 *
 * @brief Exception handling in the kernel.
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>

//
//
//
//
//
//
// FIXME this should be in arch/x86/
//
//
//
//
//
//

#include <arch/x86/exn.h>

void kernel_exn_handler(struct x86_exn_args *args) {
  struct x86_iret_stack *iret = &args->iret;
  struct x86_pusha_stack *pusha = &args->pusha;
  struct x86_exn *exn = &x86_exceptions[args->vector];

  ERROR("\n"
  "-------------------------------------------------------------------\n"
  "%d %s %s (cause: %s)\n"
  "-------------------------------------------------------------------\n"
  "eip: 0x%08x\n"
  "ebp: 0x%08x\n"
  "\n"
  "edi: 0x%08x esi: 0x%08x\n"
  "eax: 0x%08x ebx: 0x%08x\n"
  "ecx: 0x%08x edx: 0x%08x\n"
  "\n"
  "cr0: 0x%08x\n"
  "cr2: 0x%08x\n"
  "cr3: 0x%08x\n"
  "cr4: 0x%08x\n"
  "\n"
  "ds: 0x%08x\n"
  "es: 0x%08x\n"
  "fs: 0x%08x\n"
  "gs: 0x%08x\n"
  "\n"
  "error: %d\n"
  "-------------------------------------------------------------------\n",
    exn->vector, exn->mnemonic, exn->description, exn->cause,
    iret->eip,
    pusha->ebp,
    pusha->edi, pusha->esi,
    pusha->eax, pusha->ebx,
    pusha->ecx, pusha->edx,
    args->cr0,
    args->cr2,
    args->cr3,
    args->cr4,
    args->ds,
    args->es,
    args->fs,
    args->gs,
    args->error_code
  );

  panic("Exception %d during boot. Aborting.", exn->vector);
}

int exn_init(void) {
  x86_exn_set_handler(kernel_exn_handler);
  return 0;
}
