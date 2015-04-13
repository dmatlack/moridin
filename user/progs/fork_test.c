#include <syscall.h>

/* chain fork a large number of children. */
int main(int argc, char **argv)
{
	int children_remaining = 100;
	int is_child = 1, ret;

	(void) argc; (void) argv;

	while (is_child && children_remaining) {
		--children_remaining;

		ret = fork();
		if (ret < 0)
			goto out;

		is_child = !ret;
	}

out:
	return 0;
}
