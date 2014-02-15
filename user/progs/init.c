#include <syscall.h>

int global_data;

void print(const char *s) {
  int l;
  for (l = 0; s[l]; l++);
  write(0, (char *) s, l);
}

int main(int argc, char **argv) {
  int i;

  print("Hello from userspace! :)\n");

  for (i = 0; i < argc; i++) {
    print("arg: ");
    print(argv[i]);
    print("\n");
  }

  if (argv[argc] != (char *) 0) {
    print("INIT ERROR: argv[argc] != NULL!\n");
  }

  return 0;
}
