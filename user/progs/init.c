int global_data;

extern int syscall(void);

int main(int argc, char **argv) {
  (void) argc;
  (void) argv;

  syscall();

  while (1) global_data++;
}
