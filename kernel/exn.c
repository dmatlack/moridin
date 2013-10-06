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


void kernel_exn_dump(struct x86_exn_args *exn, printf_f p) {
  struct x86_pusha_stack *pusha = &exn->pusha;
  struct x86_iret_stack *iret = &exn->iret;

  p("-------------------------------------------------------------------\n");
  x86_exn_print(exn->vector, p);
  p("-------------------------------------------------------------------\n");
#define REG_FMT "0x%08x"
  p("eip: "REG_FMT"\n", iret->eip);
  p("esp: "REG_FMT"\n", iret->esp);
  p("ebp: "REG_FMT"\n", pusha->ebp);
  p("\n");
  p("edi: "REG_FMT" esi: "REG_FMT"\n", pusha->edi, pusha->esi);
  p("eax: "REG_FMT" ebx: "REG_FMT"\n", pusha->eax, pusha->ebx);
  p("ecx: "REG_FMT" edx: "REG_FMT"\n", pusha->ecx, pusha->edx);
#undef REG_FMT
  p("-------------------------------------------------------------------\n");

}

/**
 * @brief The kernel's global exception handler for the x86 architecture.
 */
void kernel_x86_exn_handler(struct x86_exn_args *exn) {

  kernel_exn_dump(exn, kprintf);

  while (1);
}
