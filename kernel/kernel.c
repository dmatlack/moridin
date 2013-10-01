/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>

#include <x86/io.h>
#include <dev/vga.h>

#include <kernel/interrupts.h>

#include <x86/exn.h> //TODO remove me

#define array_size(a) (sizeof(a) / sizeof(a[0]))

void print_exn(void) {
  int i;
  int size;

  size = array_size(x86_exceptions);
  for (i = 0; i < size; i++) {
    struct x86_exn *exn = x86_exceptions + i;

    dprintf("0x%x [%s]: %s (%s)\n", exn->vector, exn->mnemonic, 
        exn->description, exn->cause);
  }

}

void kernel_main() {

  /* initialize the Video Graphics Array so we can start printing */
  if (vga_init()) {
    panic("Unable to initialize the VGA device.\n");
  }

  /* intialize interrupt handlers */
  if (interrupts_init()) {
    panic("Unable to initialize the interrupts handlers.\n");
  }

  // kernel printing
  kprintf("Hello World!\n");
  // debug printing
  dprintf("Hello World!\n");

  print_exn();

  while (1) iodelay();
}
