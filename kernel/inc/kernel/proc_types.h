#ifndef __KERNEL_PROC_TYPES_H__
#define __KERNEL_PROC_TYPES_H__

#include <lib/list.h>

struct thread;
struct process;

list_typedef(struct thread) thread_list_t;
list_typedef(struct process) process_list_t;

#endif /* !__KERNEL_PROC_TYPES_H__ */
