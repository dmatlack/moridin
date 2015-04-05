#include <syscall.h>

int global_data;

void print(const char *s)
{
	int l;
	for (l = 0; s[l]; l++);
	write(0, (char *) s, l);
}

void print_int(int i)
{
	// Longest int string: -2147483648 (aka -2^31)
	//    = 12 characters ([-]{1}[0-9]{10} + NULL terminator)
	char string[12];
	char *s = string + 11;
	int negative;

	if ((negative = (i < 0))) {
		i *= -1;
	}

	*s = (char) 0;

	if (0 == i) {
		s--;
		*s = '0';
	}
	else {
		while (i > 0) {
			s--;
			*s = '0' + (i % 10);
			i /= 10;
		}
	}

	if (negative) {
		s--;
		*s = '-';
	}

	print(s);
}

int main(int argc, char **argv)
{
	int pid;
	int child_pid;
	int i;

	pid = getpid();
	print_int(pid);
	print(": Hello from userspace! :)\n");

	for (i = 0; i < argc; i++) {
		print("arg: ");
		print(argv[i]);
		print("\n");
	}

	if (argv[argc] != (char *) 0) {
		print("INIT ERROR: argv[argc] != NULL!\n");
	}

	child_pid = fork();

	if (child_pid < 0) {
		print("fork() failed: ");
		print_int(child_pid);
		print("\n");
	}
	else if (child_pid == 0) {
		print("CHILD: my pid = ");
		print_int(getpid());
		print(", parent pid = ");
		print_int(pid);
		print("\n");
	}
	else {
		print("PARENT: my pid = ");
		print_int(getpid());
		print(", child pid = ");
		print_int(child_pid);
		print("\n");

		yield();
	}

	return 0;
}
