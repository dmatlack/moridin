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

  while (1) iodelay();
}
