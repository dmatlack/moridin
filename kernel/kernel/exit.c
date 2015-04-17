/**
 * @file kernel/exit.h
 */
#include <kernel/syscall.h>
#include <kernel/proc.h>
#include <kernel/sched.h>

void sys_exit(int status)
{
	struct process *process = CURRENT_PROCESS;
	struct thread *thread = CURRENT_THREAD;

	INFO("Thread %d:%d exited %d.", process->pid, thread->tid, status);

	/*
	 * Assume this is the only thread in the process. Otherwise, here
	 * we'd need to kick all other threads and force them to exit.
	 */
	ASSERT_EQUALS(1, num_threads(process));

	vm_space_destroy(&process->space);

	thread->state = EXITED;
	reschedule();
}
