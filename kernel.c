/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <x86/io.h>
#include <dev/vga.h>
#include <stdio.h>

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

void delay(void) {
  int i;
  for (i = 0; i < 500000; i++) {
    iodelay();
  }
}
 
void kernel_main() {
  int i;

  vga_init();

  for (i = 0; i < 10000; i++) {
    if (i % 2 == 0) {
      printf("%s", "Hello World!");
    }
    else {
      printf("Hello 127.0.0.1!");
    }
    //delay();
  }

}
