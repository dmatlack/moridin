/**
 * @file kernel/exit.h
 */
#include <kernel/syscall.h>
#include <kernel/proc.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>
#include <kernel/init.h>
#include <kernel/wait.h>
#include <kernel/log.h>
#include <lib/errno.h>
#include <lib/assert.h>

struct spinlock process_lock = INITIALIZED_SPINLOCK;

void reparent_children(struct process *process)
{
	process_list_t *children = &process->children;
	struct process *new_parent = &init_proc;

	while (!list_empty(children)) {
		struct process *child = list_dequeue(children, sibling_link);

		add_child_process(new_parent, child);
	}

	/*
	 * The new parent may be waiting for children to exit, and one of
	 * our children may have exited.
	 */
	kick(&new_parent->wait);
}

void process_exit(int status)
{
	struct process *process = CURRENT_PROCESS;
	struct thread *thread = CURRENT_THREAD;
	unsigned long flags;

	spin_lock_irq(&process_lock, &flags);

	thread->state = EXITED;
	process->status = status;
	reparent_children(process);
	/* our parent may be waiting for us to exit */
	kick(&process->parent->wait);

	spin_unlock_irq(&process_lock, flags);

	reschedule();
}

void sys_exit(int status)
{
	struct process *p = CURRENT_PROCESS;

	INFO("Thread %d:%d exited %d.", p->pid, CURRENT_THREAD->tid, status);

	/*
	 * Assume this is the only thread in the process. Otherwise, here
	 * we'd need to kick all other threads and force them to exit.
	 * Everything below this point assumes we are the last thread in
	 * the process to exit.
	 */
	ASSERT_EQUALS(1, num_threads(p));
	ASSERT_NOTEQUALS(1, p->pid);

	vm_space_destroy(&p->space);

	vfs_file_put(p->exec_file);

	process_exit(status);
}

struct process *find_exited(process_list_t *processes)
{
	struct process *p;

	list_foreach(p, processes, sibling_link) {
		struct thread *t;
		bool exited = true;

		list_foreach(t, &p->threads, thread_link) {
			if (t->state == EXITED)
				continue;

			exited = false;
			break;
		}

		if (exited)
			return p;
	}

	return NULL;
}

int sys_wait(int *status)
{
	struct process *process = CURRENT_PROCESS;
	process_list_t *children = &process->children;
	struct process *child;
	int ret = 0;
	unsigned long flags;

	TRACE("status=%p", status);

	spin_lock_irq(&process_lock, &flags);

	for (;;) {
		if (list_empty(children)) {
			ret = ECHILD;
			goto out;
		}

		child = find_exited(children);
		if (child)
			break;

		begin_wait(&process->wait);
		spin_unlock_irq(&process_lock, flags);

		reschedule();

		spin_lock_irq(&process_lock, &flags);
	}

	INFO("Process %d reaping child %d.", process->pid, child->pid);

	/* FIXME status is a user pointer (unsafe to dereference) */
	*status = child->status;

	while (!list_empty(&child->threads))
		free_thread_struct(list_dequeue(&child->threads, thread_link));

	list_remove(children, child, sibling_link);
	free_process_struct(child);

out:
	spin_unlock_irq(&process_lock, flags);
	return ret;
}
