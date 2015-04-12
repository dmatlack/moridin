/**
 * @file kernel/proc.h
 *
 */
#ifndef __KERNEL_PROC_H__
#define __KERNEL_PROC_H__

#include <mm/kmalloc.h>
#include <arch/atomic.h>
#include <mm/memory.h>
#include <mm/vm.h>
#include <list.h>

struct thread;
struct process;

list_typedef(struct thread) thread_list_t;
list_typedef(struct process) proc_list_t;

#include <arch/reg.h>

#define _THREAD(stack_addr)        ((struct thread *) PAGE_ALIGN_DOWN(stack_addr))
#define _PROC(stack_addr)          ((_THREAD(stack_addr))->proc)

#define CURRENT_THREAD             _THREAD(get_sp())
#define CURRENT_PROC               _PROC(get_sp())

#define KSTACK_SIZE 2048

#define _KSTACK_START(_thread)    ((unsigned long) (_thread)->kstack)
#define _KSTACK_END(_thread)      (_KSTACK_START(_thread) + KSTACK_SIZE)
#define _KSTACK_TOP(_thread)      (_KSTACK_END(_thread) - sizeof(void*))

#define KSTACK_START              _KSTACK_START(CURRENT_THREAD)
#define KSTACK_END                _KSTACK_END(CURRENT_THREAD)
#define KSTACK_TOP                _KSTACK_TOP(CURRENT_THREAD)

#define THREAD_STRUCT_ALIGN PAGE_SIZE
struct thread {
	char			kstack[KSTACK_SIZE];
	struct process    *	proc;
	struct registers  *	regs;
	void *			context;
	int			tid;
	int			preempt;
#define RESCHEDULE	0x1 /* the thread has been preempted */
	u64			flags;

	list_link(struct thread) thread_link;
	list_link(struct thread) sched_link;

} __attribute__((aligned (THREAD_STRUCT_ALIGN)));

static inline bool check_flags(u64 mask)
{
	return CURRENT_THREAD->flags & mask;
}

static inline void set_flags(u64 mask)
{
	CURRENT_THREAD->flags |= mask;
}

static inline void clear_flags(u64 mask)
{
	CURRENT_THREAD->flags &= ~mask;
}

struct process {
	struct process    *parent;
	proc_list_t        children;
	thread_list_t      threads;
	struct vm_space    space;
	int                next_tid;
	int                pid;

	list_link(struct process) sibling_link;

};

#define num_threads(_proc) (list_size(&(_proc)->threads))
#define main_thread(_proc) (list_head(&(_proc)->threads))

/**
 * @brief Allocate an initialize a new thread struct.
 */
static inline struct thread *new_thread_struct() {
	struct thread *t;

	t = kmemalign(THREAD_STRUCT_ALIGN, sizeof(struct thread));
	if (t) {
		memset(t, 0, sizeof(struct thread));
	}
	return t;
}

static inline void free_thread_struct(struct thread *t) {
	kfree(t, sizeof(struct thread));
}

int next_pid(void);

/**
 * @brief Allocate and initialize a new process struct.
 */
static inline struct process *new_process_struct()
{
	struct process *p;

	p = kmalloc(sizeof(struct process));
	if (p) {
		list_init(&p->children);
		list_init(&p->threads);
		list_elem_init(p, sibling_link);
		p->pid = next_pid();
		p->next_tid = 0;
	}
	return p;
}

static inline void free_process_struct(struct process *p)
{
	kfree(p, sizeof(struct process));
}

static inline void add_thread(struct process *p, struct thread *t)
{
	list_insert_tail(&p->threads, t, thread_link);
	t->proc = p;
	t->tid = atomic_add(&p->next_tid, 1);
}

static inline void add_child_process(struct process *parent, struct process *child)
{
	child->parent = parent;
	list_insert_tail(&parent->children, child, sibling_link);
}

#endif /* !_KERNEL_PROC_H__ */
