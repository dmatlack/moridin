int global_data;

extern int syscall(int s, void *arg);

int main(int argc, char **argv) {
  (void) argc;
  (void) argv;

  syscall(0, (void *) "Hello from userspace!\n");

  while (1) global_data++;
}
