#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <moridin/syscall.h>

#define CHECK(_condition) do {						\
	int __condition = (_condition);					\
									\
	if (__condition)						\
		break;							\
									\
	printf("FAILED: %s [%d]\n", #_condition, __condition);		\
	exit(42);							\
} while (0)

#define NUM_CHILDREN 100

void run_parent(void)
{
	int children_reaped = 0;
	int status;

	for (; children_reaped < NUM_CHILDREN; children_reaped++) {
		int ret;

		for (;;) {
			ret = wait(&status);
			if (!ret)
				break;

			yield();
		}

		printf("wait(): ret %d status %d\n", ret, status);
	}

	/* we should have no children left to reap */
	CHECK(wait(&status) < 0);
}

void run_child(void)
{
	int children_created = 1; /* we're already a child */

	for (; children_created < NUM_CHILDREN; children_created++) {
		int ret = fork();
		CHECK(ret >= 0);

		/* Each process forks once then exits. */
		if (ret)
			break;
	}
}

/* chain fork a large number of children. */
int main(int argc, char **argv)
{
	(void) argc; (void) argv;
	int ret;

	ret = fork();
	CHECK(ret >= 0);

	if (ret)
		run_parent();
	else
		run_child();

	return 0;
}
