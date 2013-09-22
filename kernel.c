
/* Check if the compiler thinks if we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#include <x86/io.h>
#include <dev/vga.h>

void print(const char *s) {
  for (; s && *s; s++) {
    vga_putbyte(*s);
  }
}

void delay(void) {
  int i;
  for (i = 0; i < 500000; i++) {
    iodelay();
  }
}
 
void kernel_main() {
  int i = 0;

  vga_init();

  for (i = 0; i < 10000; i++) {
    if (i % 2 == 0) {
      print("Hello World!");
    }
    else {
      print("Hello 127.0.0.1!");
    }
    //delay();
  }

}
