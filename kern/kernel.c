/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>

#include <x86/io.h>
#include <dev/vga.h>

void kernel_main() {

  // initialize the Video Graphics Array so we can start printing
  must_succeed( vga_init() );

  // intialize the Programmable Interrupt Controller
  must_succeed( pic_init(0x20, 0x28) );


  // kernel printing
  kprintf("Hello World!\n");
  // debug printing
  dprintf("Hello World!\n");

  while (1) iodelay();
}
