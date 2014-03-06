/**
 * @file kernel/fork.c
 *
 * @brief Implementation of the fork system call.
 *
 */
#include <kernel/syscall.h>
#include <kernel/proc.h>
#include <kernel/kmalloc.h>

#include <types.h>
#include <errno.h>

pid_t sys_fork(void) {

  if (num_threads(CURRENT_PROC) > 1) {
    /*
     * For POSIX specification on multithreaded fork see:
     * http://pubs.opengroup.org/onlinepubs/000095399/functions/fork.html
     */
    DEBUG("Multithreaded fork() not supported at the moment.");
    return -1;
  }

  panic("unimplemented");
}
