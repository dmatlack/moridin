#include <syscall.h>

int global_data;

int main(int argc, char **argv) {
  (void) argc;
  (void) argv;

  __syscall(0, (void *) "Hello from userspace!\n");

  while (1) global_data++;
}
