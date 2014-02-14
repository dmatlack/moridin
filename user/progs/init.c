#include <syscall.h>

int global_data;

void print(const char *s) {
  int l;
  for (l = 0; s[l]; l++);
  write(0, (char *) s, l);
}

int main(int argc, char **argv) {
  (void) argc;
  (void) argv;

  while (1) {
    print("Hello from userspace!\n");
  }

}
