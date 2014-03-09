/**
 * @file kernel/sched.c
 */
#include <kernel/sched.h>

static thread_list_t runnable;

void schedule(struct thread *thread) {
  list_insert_tail(&runnable, thread, sched_link);
}
