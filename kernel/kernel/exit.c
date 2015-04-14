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

	thread->state = EXITED;
	reschedule();
}
