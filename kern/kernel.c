/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <x86/io.h>
#include <dev/vga.h>
#include <kern/kprintf.h>
#include <x86/idtr.h>

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

void delay(void) {
  int i;
  for (i = 0; i < 500000; i++) {
    iodelay();
  }
}

void kernel_main(uint32_t esp, uint32_t idt) {

  vga_init();

  kprintf("Hello World!\n");

  while (1) {
    delay();
  }
}
