/**
 * @file kernel/sched.c
 */
#include <kernel/sched.h>
#include <arch/sched.h>
#include <arch/irq.h>
#include <list.h>

static thread_list_t runnable;

void sched_make_runnable(struct thread *thread) {
  list_enqueue(&runnable, thread, sched_link);
}

void sched_switch(void) {
  struct thread *next = list_dequeue(&runnable, sched_link);

  context_switch(next);
}
