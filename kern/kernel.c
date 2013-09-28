/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>

#include <x86/io.h>
#include <dev/vga.h>

void delay(void) {
  int i;
  for (i = 0; i < 500000; i++) {
    iodelay();
  }
}

void kernel_main() {

  vga_init();

  kprintf("Hello World!\n");
  dprintf("Hello Bochs Debug Console!\n");

  while (1) {
    delay();
  }
}
