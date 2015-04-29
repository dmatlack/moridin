#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int child_pid;
	int i;

	printf("%d: Hello from userspace! :)\n", getpid());

	for (i = 0; i < argc; i++)
		printf("arg %d: %s\n", i, argv[i]);

	child_pid = fork();

	if (child_pid < 0)
		printf("fork() failed: %d\n", child_pid);

	return 0;
}
